#include "stdafx.h"
#include "../include/rs485monitor.h"

static const char* TAG = "485Monitor";

RS485Monitor::RS485Monitor()
{

}

void RS485Monitor::OnCreate()
{
    LoadConfig();
}

void RS485Monitor::LoadConfig()
{
    mItems.clear();

    auto filePath = ShellTool::GetAppPath() + "/rs485.json";
    ByteBuffer box;
    File::ReadFile(filePath, box);
    if (box.length() > 3)
    {
        auto d = box.data();
        if (d[0] == 0xEF && d[1] == 0xBB && d[2] == 0xBF)
        {
            //remove utf8 BOM
            box.Eat(3);
        }
    }

    if (!box.empty())
    {


        DynamicJsonBuffer jBuffer;
        auto& json = jBuffer.parseObject(box.data());
        if (json.success())
        {
            auto& sensors = json["sensors"].as<JsonArray>();
            for (auto& item : sensors)
            {
                BYTE addr = item["addr"].as<BYTE>();
                tagSensorItem sItem;
                sItem.addr = addr;
                sItem.title= item["title"].as<string>();
                mItems[addr] = sItem;
            }
        }
    }

}

tagSensorItem *RS485Monitor::item(BYTE addr)
{
    auto it = mItems.find(addr);
    if (it == mItems.end())
    {
        return nullptr;
    }

    return &it->second;
}

/*
解析出不认识的数据
*/
void RS485Monitor::OnParsedUnknown(LPBYTE data, int bytes)
{
    auto hex = ByteTool::ByteToHexChar(data, bytes);
    LogV(TAG, "%s[%s]", __func__, hex.c_str());

    SignalUnknownDataReady(data, bytes);
}

/* crc已检验ok */
void RS485Monitor::OnParsedFrame(LPBYTE frame, int bytes)
{
    auto hex = ByteTool::ByteToHexChar(frame, bytes);
    LogV(TAG, "%s[%s]",__func__,hex.c_str());

    SignalFrameReady(frame, bytes);
}

#if 0
[09.02 10:40 : 54.807]1D 03 00 00 00 04 46 55 1D 03 08 00 FC 00 02 01 14 00 01 3B B0
485 crc不匹配
[09.02 10:41 : 53.254]0E 03 00 00 00 04 44 F6 0E 03 08 03 16 00 02 01 0C 00 01 EB E8
485 crc不匹配
[09.02 10:41 : 53.577]18 03 00 00 00 04 46 00 18 03 08 00 6E 00 01 01 0F 00 01 AD B2
485 crc不匹配
[09.02 10:41 : 53.900]19 03 00 00 00 04 47 D1 19 03 08 13 0B 00 02 01 12 00 01 09 57
#endif

void RS485Monitor::Input(LPBYTE d, int bytes)
{
    ByteBuffer box;
    box.Write((LPVOID)d, bytes);

    while (1)
    {
        auto bytes = box.length();
        ParseBox(box);

        if (bytes == box.length())
        {
            break;
        }
    }

    if (!box.empty())
    {
        OnParsedUnknown(box.data(), box.length());
    }
}

/*
逐字节解析出crc匹配的数据
*/
void RS485Monitor::ParseBox(ByteBuffer& box)
{
    const int minPackBytes = 8;
    const int maxPackBytes = 64;

    auto maxOffset = box.length() - minPackBytes;
    for (int offset = 0; offset <= maxOffset;++offset)
    {
        for (int len = minPackBytes; len <= maxPackBytes; len++)
        {
            /* 检查从offset开始的len字节 */
            auto ps = box.data()+offset;

            if (offset + len > box.length())
            {
                break;
            }

            auto ok=Crc16::CrcMatched(ps, len);
            if (ok)
            {
                auto start = box.data();
                int eatBytes = 0;
                if (ps > start)
                {
                    int unknownBytes = ps - start;
                    OnParsedUnknown(start, unknownBytes);

                    eatBytes += unknownBytes;
                }
                
                OnParsedFrame(ps, len);
                eatBytes += len;

                box.Eat(eatBytes);
                return;
            }
        }
    }
}
