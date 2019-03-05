#include "stdafx.h"
#include "httprequesthandler_file.h"
#include "file/virtualfolder.h"
#include "httptool.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
using namespace FileSystem;
namespace Net {
namespace Http {

NameValue	*CHttpRequestHandler_File::m_mapUriFile = NULL;

CHttpRequestHandler_File::CHttpRequestHandler_File()
{
	m_hFile = NULL;
	m_fileSize = 0;
	m_fileOffset = 0;
}

CHttpRequestHandler_File::~CHttpRequestHandler_File()
{
	if (m_hFile)
	{
		fclose(m_hFile);
		m_hFile = NULL;
	}
}

bool CHttpRequestHandler_File::IsSending()
{
	if (!m_hFile)
	{
		return false;
	}

	if (m_fileOffset < m_fileSize)
	{
		return true;
	}

	return false;
}

/*
http range bytes是[min,max]
比如bytes=0-100时服务器会传101字节
已确认min是从0开始的
*/
int CHttpRequestHandler_File::Process()
{
	ASSERT(GetStatus() == eHttpHandlerStatus_Processing);

	auto fileRange = m_fileSize;
	if (m_headerInfo->m_rangeEnd > 0)
	{
		//ASSERT(m_headerInfo->m_rangeEnd < mFileSize);
		fileRange = m_headerInfo->m_rangeEnd + 1;
	}

	if (m_fileOffset < fileRange)
	{
		BYTE data[4 * 1024];
		auto maxRead = (int)(MIN(sizeof(data), fileRange - m_fileOffset));
		if (m_outbox->GetFreeSize() >= maxRead)
		{
			auto ret = (int)fread(data, 1, maxRead, m_hFile);
			if (ret > 0)
			{
				m_fileOffset += ret;

				Output(data, ret);
			}
			else
			{
				DW("fail fread,ret=%d", ret);
			}
		}
	}

	if (m_fileOffset >= fileRange)
	{
		SetStatus(eHttpHandlerStatus_Done);
	}

	return 0;
}

int CHttpRequestHandler_File::Start(tagHttpHeaderInfo *headerInfo)
{
	ASSERT(!m_hFile);
	ASSERT(m_fileSize == 0);
	ASSERT(m_fileOffset == 0);

	HttpRequestHandler::Start(headerInfo);
	SetStatus(eHttpHandlerStatus_Processing);

	//处理普通文件
	FILE *hFile = NULL;
	string  szFileName;
	string  uri = m_headerInfo->m_uri;
	shared_ptr<VirtualFolder> vm = mWebConfig->mVirtualFolder;

	if (m_mapUriFile)
	{
		string  filepath = m_mapUriFile->GetString(uri.c_str()).c_str();
		if (!filepath.empty() && File::FileExists(filepath.c_str()))
		{
			if (headerInfo->m_curUserLevel == eUser_Admin)
			{
				ASSERT(!hFile);
				hFile = File::fopen(filepath.c_str(), "rb");
			}
			else
			{
				return OnFileNoAuth();
			}
		}
	}

	//todo:屏蔽uri中的..字样,防止恶意用户非法访问文件

	//先试虚拟目录文件
	if (!hFile && vm)
	{
		if (uri.empty())
		{
			uri = "/";
		}
		else if (uri[0] != '/')
		{
			uri = "/" + uri;
		}

		string  localPath = vm->Virtual2LocalPathFile(uri);
		if (!localPath.empty())
		{
			char filename[MAX_PATH];
			memset(filename, 0, sizeof(filename));
			strncpy(filename, localPath.c_str(), sizeof(filename) - 1);
			File::PathMakePretty(filename);
			szFileName = filename;

			ASSERT(!hFile);
			hFile = File::fopen(szFileName.c_str(), "rb");
		}
	}

	if (!hFile)
	{
		//再试www文件夹
		szFileName = StringTool::Format("%s%s", mWebConfig->mWebRootFolder.c_str(), uri.c_str());
		hFile = File::fopen(szFileName.c_str(), "rb");
	}

	if (!hFile)
	{
		return OnFileNoFound(uri);
	}

	m_hFile = hFile;
	m_fileSize = File::GetFileLength(hFile);
	m_fileOffset = 0;

	string  ext = HttpTool::GetUriExt(uri);
	string  contentType = HttpTool::GetContentType(ext);

	string  szLastModifiedTime;
	bool canUseCache = HttpTool::CheckFileCache(szFileName, m_headerInfo->m_if_modified_since,
		szLastModifiedTime);
	if (canUseCache)
	{
		fclose(m_hFile);
		m_hFile = NULL;
		m_fileSize = 0;

		CHttpAckHeader header;
		header.SetStatusCode("304 Not Modified");
		header.SetContentType(contentType);
		header.SetConnection("Keep-Alive");
#ifdef _DEBUG		
		header.SetCacheControl("max-age=5");
#else
		header.SetCacheControl("max-age=60");
#endif

		string  ack = header.ack();
		/*
		ack.Format(
			"HTTP/1.1 304 Not Modified\r\n"
			"Content-Type: %s\r\n"
			FILE_CACHE_MAX_AGE
			"Connection: Keep-Alive\r\n"
			"\r\n"
			,
			contentType.c_str()
			);
			//*/

		Output(ack);
		SetStatus(eHttpHandlerStatus_Done);
		return 0;
	}

	string  header;
	//注意:下载ocx时,不能使用Cache-Control: no-store
	//否则IE会提示不能保存文件
	if (IsPartialContent())
	{
		if (m_headerInfo->m_rangeEnd <= 0)
		{
			m_headerInfo->m_rangeEnd = MAX(0, m_fileSize - 1);
		}

		if (m_headerInfo->m_rangeEnd >= m_headerInfo->m_range)
		{
			auto contentLength = m_headerInfo->m_rangeEnd - m_headerInfo->m_range + 1;

			CHttpAckHeader httpAckHeader;
			httpAckHeader.SetStatusCode("206 Partial Content");
			httpAckHeader.SetContentLength(contentLength);
			httpAckHeader.SetContentType(contentType);
			httpAckHeader.SetConnection("Keep-Alive");

			string  contentRange = StringTool::Format("bytes %lu-%lu/%lu",
				m_headerInfo->m_range,
				m_headerInfo->m_rangeEnd,
				m_fileSize
			);
			httpAckHeader.SetContentRange(contentRange);
			header = httpAckHeader.ack();

			/*
			header.Format(
				"HTTP/1.1 206 Partial Content\r\n"
				"Content-Length: %lu\r\n"
				"Content-Type: %s\r\n"
				"Content-Range: bytes %lu-%lu/%lu\r\n"
				"Connection: Keep-Alive\r\n"
				"\r\n"
				,
				dwRangeSize,
				contentType.c_str(),
				range,
				m_fileSize-1,
				m_fileSize
				);
			//*/

			m_fileOffset = m_headerInfo->m_range;
			fseek(m_hFile, (long)m_fileOffset, SEEK_SET);
		}
		else
		{
			CHttpAckHeader httpAckHeader;
			httpAckHeader.SetStatusCode("416 Requested Range Not Satisfiable");
			httpAckHeader.SetContentLength(m_fileSize);
			httpAckHeader.SetContentType(contentType);
			httpAckHeader.SetConnection("Keep-Alive");

			header = httpAckHeader.ack();
			/*
			header.Format(
				"HTTP/1.1 416 Requested Range Not Satisfiable\r\n"
				"Content-Length: %lu\r\n"
				"Content-Type: %s\r\n"
				"Connection: Keep-Alive\r\n"
				"\r\n"
				,
				m_fileSize,
				contentType.c_str()
				);
				//*/

			fclose(m_hFile);
			m_hFile = NULL;
			m_fileSize = 0;
		}
	}
	else
	{
		CHttpAckHeader httpAckHeader;
		httpAckHeader.SetStatusCode("200 OK");
		if (ext == ".avi" || ext == ".mp4")
		{
			httpAckHeader.SetField("Content-Disposition", "attachment");//suggest browser save file
		}

		httpAckHeader.SetContentType(contentType);
		httpAckHeader.SetContentLength(m_fileSize);
		httpAckHeader.SetConnection("Keep-Alive");
		httpAckHeader.SetField("Last-Modified", szLastModifiedTime);

		header = httpAckHeader.ack();

		/*
		header.Format(
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: %lu\r\n"
			"Content-Type: %s\r\n"
			"Connection: Keep-Alive\r\n"
			"\r\n"
			//"Connection: close\r\n"
			//"Cache-Control: no-cache,no-store,must-revalidate\r\n"
			,
			m_fileSize,
			contentType.c_str()
			);
			//*/
	}

	ASSERT(!header.empty());

	Output(header);
	return Process();//发第一块文件数据
}

int CHttpRequestHandler_File::OutputPlainText(string  text, string  statusCode)
{
	string  content = StringTool::Format(
		"<html><body>%s</body></html>",
		text.c_str()
	);

	CHttpAckHeader httpAckHeader;
	httpAckHeader.SetStatusCode(statusCode);//"200 OK");
	httpAckHeader.SetContentType("text/html;charset=UTF-8");
	httpAckHeader.SetContentLength((int)content.length());
	httpAckHeader.SetConnection("Keep-Alive");
	httpAckHeader.SetCacheControl("no-cache,no-store");

	string  ack = StringTool::Format(
		"%s"
		"%s"
		,
		httpAckHeader.ack().c_str(),
		content.c_str()
	);
	/*
	ack.Format(
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html;\r\n"
		"Connection: Close\r\n"
		"Cache-Control: no-cache,no-store\r\n"
		"Content-Length: %d\r\n"
		"\r\n"
		"%s"
		,
		content.length(),
		content.c_str()
	);
	//*/

	Output(ack);
	SetStatus(eHttpHandlerStatus_Done);
	return 0;
}

int CHttpRequestHandler_File::OnFileNoAuth()
{
	return OutputPlainText("No Auth", "401 No Auth");
}

int CHttpRequestHandler_File::OnFileNoFound(string  uri)
{
	string  msg = StringTool::Format("Resource not found: %s", uri.c_str());

	return OutputPlainText(msg, "404 Not Found");
}

//filepath为真实路径,不会进行虚拟目录转换
//所有uri file map都需要管理员权限才能访问
void CHttpRequestHandler_File::AddUriFileMap(string  uri, string  filepath)
{
	if (!m_mapUriFile)
	{
		m_mapUriFile = new NameValue;
	}

	m_mapUriFile->Set(uri.c_str(), filepath.c_str());
}

void CHttpRequestHandler_File::EmptyUriFileMap()
{
	delete m_mapUriFile;
	m_mapUriFile = NULL;
}

}
}
}
}