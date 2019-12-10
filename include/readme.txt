为简化app使用,约定在用到某个lib时，app只需要包含此lib的一个.inl文件即可
需要导出给app头文件都应该放在此.inl文件中

比如用到libcorelooper时，只需要#include "libcorelooper.inl"

在libcorelooper.inl内部include其他.h时要基于.inl所在的目录
比如core/base中的dt.h在包含同目录下的win32.h和shelltool.h时
本来是可以直接
#include "win32.h"
#include "shelltool.h"
但为了便于app只指定一个dir作为include的path,所以要采用如下方式
#include "core/base/win32.h"
#include "core/base/shelltool.h"
这样app在编译时也能找到win32.h和shelltool.h


注意到microsoft提供的windows sdk中.h是全部放在一个include中的，缺点是头文件显得没有层次
