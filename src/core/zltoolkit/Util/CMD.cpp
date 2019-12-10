﻿#include "stdafx.h"
/*
 * MIT License
 *
 * Copyright (c) 2016-2019 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "CMD.h"

#if defined(_WIN32)
#include "win32/getopt.h"
#else
#include <getopt.h>
#endif // defined(_WIN32)

namespace toolkit{

mutex OptionParser::s_mtx_opt;
//默认注册exit/quit/help/clear命令
static onceToken s_token([](){
    REGIST_CMD(exit)
    REGIST_CMD(quit)
    REGIST_CMD(help)
    REGIST_CMD(clear)
}, nullptr);


CMDRegister &CMDRegister::Instance(){
    static CMDRegister instance;
    return instance;
}

void OptionParser::operator ()(mINI &allArg, int argc, char *argv[],const std::shared_ptr<ostream> &stream) {
    vector<struct option> vec_longOpt;
    string str_shortOpt;
    do{
        struct option tmp;
        for (auto &pr : _map_options) {
            auto &opt = pr.second;
            //long opt
            tmp.name = (char *) opt._longOpt.data();
            tmp.has_arg = opt._argType;
            tmp.flag = NULL;
            tmp.val = pr.first;
            vec_longOpt.emplace_back(tmp);
            //short opt
            if (!opt._shortOpt) {
                continue;
            }
            str_shortOpt.push_back(opt._shortOpt);
            switch (opt._argType) {
                case Option::ArgRequired:
                    str_shortOpt.append(":");
                    break;
                case Option::ArgOptional:
                    str_shortOpt.append("::");
                    break;
                default:
                    break;
            }
        }
        tmp.flag = 0;
        tmp.name = 0;
        tmp.has_arg = 0;
        tmp.val = 0;
        vec_longOpt.emplace_back(tmp);
    }while(0);

    lock_guard<mutex> lck(s_mtx_opt);
    int index;
    optind = 0;
    opterr = 0;
    while ((index = getopt_long(argc, argv, &str_shortOpt[0], &vec_longOpt[0],NULL)) != -1) {
        stringstream ss;
        ss  << "  未识别的选项,输入\"-h\"获取帮助.";
        if(index < 0xFF){
            //短参数
            auto it = _map_charIndex.find(index);
            if(it == _map_charIndex.end()){
                throw std::invalid_argument(ss.str());
            }
            index = it->second;
        }

        auto it = _map_options.find(index);
        if(it == _map_options.end()){
            throw std::invalid_argument(ss.str());
        }
        auto &opt = it->second;
        auto pr = allArg.emplace(opt._longOpt, optarg ? optarg : "");
        if (!opt(stream, pr.first->second)) {
            return;
        }
        optarg = NULL;
    }
    for (auto &pr : _map_options) {
        if(pr.second._defaultValue && allArg.find(pr.second._longOpt) == allArg.end()){
            //有默认值,赋值默认值
            allArg.emplace(pr.second._longOpt,*pr.second._defaultValue);
        }
    }
    for (auto &pr : _map_options) {
        if(pr.second._mustExist){
            if(allArg.find(pr.second._longOpt) == allArg.end() ){
                stringstream ss;
                ss << "  参数\"" << pr.second._longOpt << "\"必须提供,输入\"-h\"选项获取帮助";
                throw std::invalid_argument(ss.str());
            }
        }
    }
    if(allArg.empty() && _map_options.size() > 1 && !_enableEmptyArgs){
        _helper(stream,"");
        return;
    }
    if (_onCompleted) {
        _onCompleted(stream, allArg);
    }
}

}//namespace toolkit