//
// Created by xzl on 2019/6/28.
//

#ifndef ZLMEDIAKIT_PARSER_H
#define ZLMEDIAKIT_PARSER_H

#include <map>
#include <string>
//#include "Util/util.h"
#include "core/string/stringcasemap.h"
using namespace std;
//using namespace toolkit;

#define strcasecmp _stricmp
vector<string> split(const string& s, const char* delim);

namespace mediakit{
using namespace Bear::Core;
string FindField(const char *buf, const char *start, const char *end, int bufSize = 0);


class Parser {
    public:
    Parser() {}

    virtual ~Parser() {}

    void Parse(const char *buf) {
        //解析
        const char *start = buf;
        Clear();
        while (true) {
            auto line = FindField(start, NULL, "\r\n");
            if (line.size() == 0) {
                break;
            }
            if (start == buf) {
                _strMethod = FindField(line.data(), NULL, " ");
                _strFullUrl = FindField(line.data(), " ", " ");
                auto args_pos = _strFullUrl.find('?');
                if (args_pos != string::npos) {
                    _strUrl = _strFullUrl.substr(0, args_pos);
                    _params = _strFullUrl.substr(args_pos + 1);
                    _mapUrlArgs = parseArgs(_params);
                } else {
                    _strUrl = _strFullUrl;
                }
                _strTail = FindField(line.data(), (_strFullUrl + " ").data(), NULL);
            } else {
                auto field = FindField(line.data(), NULL, ": ");
                auto value = FindField(line.data(), ": ", NULL);
                if (field.size() != 0) {
                    _mapHeaders.emplace(field,value);
                }
            }
            start = start + line.size() + 2;
            if (strncmp(start, "\r\n", 2) == 0) { //协议解析完毕
                _strContent = FindField(start, "\r\n", NULL);
                break;
            }
        }
    }

    const string &Method() const {
        //rtsp方法
        return _strMethod;
    }

    const string &Url() const {
        //rtsp url
        return _strUrl;
    }

    const string &FullUrl() const {
        //rtsp url with args
        return _strFullUrl;
    }

    const string &Tail() const {
        //RTSP/1.0
        return _strTail;
    }

    const string &operator[](const char *name) const {
        //rtsp field
        auto it = _mapHeaders.find(name);
        if (it == _mapHeaders.end()) {
            return _strNull;
        }
        return it->second;
    }

    const string &Content() const {
        return _strContent;
    }

    void Clear() {
        _strMethod.clear();
        _strUrl.clear();
        _strFullUrl.clear();
        _params.clear();
        _strTail.clear();
        _strContent.clear();
        _mapHeaders.clear();
        _mapUrlArgs.clear();
    }
    const string &Params() const {
        return _params;
    }

    void setUrl(const string &url) {
        this->_strUrl = url;
    }

    void setContent(const string &content) {
        this->_strContent = content;
    }

    StringCaseMap &getValues() const {
        return _mapHeaders;
    }

    StringCaseMap &getUrlArgs() const {
        return _mapUrlArgs;
    }

    static StringCaseMap parseArgs(const string &str, const char *pair_delim = "&", const char *key_delim = "=") {
        StringCaseMap ret;
        auto arg_vec = split(str, pair_delim);
        for (string &key_val : arg_vec) {
            auto key = FindField(key_val.data(), NULL, key_delim);
            auto val = FindField(key_val.data(), key_delim, NULL);
            ret.emplace(key,val);
        }
        return ret;
    }

private:
    string _strMethod;
    string _strUrl;
    string _strTail;
    string _strContent;
    string _strNull;
    string _strFullUrl;
    string _params;
    mutable StringCaseMap _mapHeaders;
    mutable StringCaseMap _mapUrlArgs;
};


}//namespace mediakit

#endif //ZLMEDIAKIT_PARSER_H
