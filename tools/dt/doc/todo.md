
# 2020.02.09
- �����������������mMaxLogCount

# 2020.02.08
changes:
- ʵ����log list ctrl�Ҽ��˵��Ĺ���:open file and goto line,open folder,copy full path
- app,tag filter���
- ������ItemPage
- ������level combo

todo:
- ʵ��Ctrl+Tab�л�
- �ù����ڴ����Ż�������־������ռ��app��ʱ��

��������:
- appд��־�������ڴ�
- ����LogServer,��app������ͬһ����,LogServer����ɼ���־,��websocket�ṩ��־����
- client��websocket��ȡ��־
- ���ڴ滻ȡapp����

Ŀǰ����Ҫ��ô�ߵ����ܣ���������Ҫ����˵

# 2020.02.07

changes:
- ������open file goto line ����
  
goto line�ǲ���c#����CoreLooper\tools\OpenFileGotoLine������

<pre>
https://stackoverflow.com/questions/350323/open-a-file-in-visual-studio-at-a-specific-line-number
https://docs.microsoft.com/en-us/visualstudio/ide/reference/devenv-command-line-switches?view=vs-2019
https://docs.microsoft.com/en-us/visualstudio/ide/reference/visual-studio-commands?view=vs-2019
https://docs.microsoft.com/en-us/visualstudio/ide/reference/go-to-command?view=vs-2019


devenv "Path\To\Your\Solution" /edit "Path\To\Your\File" /command "Edit.GoTo 123"

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe"  /edit "d:\t.cpp" /Command "Edit.GoTo 12"
ֻ�ܴ��ļ������ܶ�λ����

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\dt.sln"  /edit "D:\CoreLooper\projects\dt\src\Consolepage.cpp" /Command "Edit.GoTo 12"
���ı���ʽ����dt.sln���ܶ�λ����

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\src\Consolepage.cpp"  /edit  /Command "Edit.GoTo 12"


"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\src\Consolepage.cpp"    /Command "Edit.GoTo 12"
���µ�vs���ܶ�λ����

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" "D:\CoreLooper\projects\dt\src\Consolepage.cpp"  /edit "D:\CoreLooper\projects\dt\src\Consolepage.cpp" /Command "Edit.GoTo 12"

</pre>

# 2020.02.05
filter����
- �������֮����ö��ŷָ�

����app=BearStudio,IotPlatform

Ϊ��ʵ�ַ�����������,�ֶ����ִ�Сд

- ֧��! ����app����!PlatformServerʱ������PlatformServer����־





# 2020.02.04
����Ŀǰ��DTû��������Ҫ��,��Ҫ��ȱ�����¹���

- ���������̵���־
   ͬʱ���ж�����̶���DT�����Ϣʱ����ʱֻ��Ҫ��עĳ������

��������DT����,Ҫ֧�����¹��ܵ�

- ���ݾɵ�dt
- ����tabҳ�棬ÿ�����̵���־����ʾ�ڸ��Ե�tab���棬���ڵ��Զ��app,����Ӱ��

ÿ����־����������Ϣ
- appName(exeName),pid,tid
- tag
- message
- log level:verbose,debug,warning,error...
- source file,line

## �������
- �ɸ��ݸ��ֲ�������
- �Ż�WM_COPYDATA���ݸ�ʽ������TLV����

## UI����
��һ����tab