# todo
CoreLooper�ѳ�������,�������¹����д�����(���������Ⱥ�)  
- дEnglish�ĵ�  
Ŀǰ�����ĵ���������֮�У��ƻ��ȿ�Դһ��ʱ�䣬��ȡ�û��ķ��������޸������ĵ�  
�������ĵ������ȶ����ٱ�дEnglish�ĵ���Ԥ��2019.6�¿�ʼ��
- ����ӿ��ع�
 Ŀǰ���ڴ���Handler��Looperϸ�ڣ���������ʱ������������ӿ�
- ����UnitTest�͸�������ʾ�÷� 
- ֧��RPC
- HTTPS
 ��Ҫ���ڷ�����TLS����,openssl��mbedtls��û���ṩ���õĽӿ�
- WebSocket Server
- ͨ�õ�dns async parser
 ��������һ��DnsLooper�ڲ���������ʽʵ��,�д��Ľ�
- Log��־
 ��Ҫ������������  
 �����ܣ���Ӱ����ҵ��  
����������ȥ��  
�ܸ�����������  
���

- Looper��
 ������CoreLooper�����ʵ��һ��Looper��Ӧ�ü򵥣�����û�뵽�õ�Ӧ�ó���
- �ع��Ż�proc.xml
 ��Ҫ����Ҫ����ð취�Է������ķ�ʽ��BlockLooper��֧�������Ҳ��ʾ��proc��,���ܿ���shadow buddy
- ͳһ������Ϣ��Ͷ��ʧ��ʱ������
 ֻ������Looper��ֹͣ����ֱ�Ӷ�Looper����Ϣ�ĳ���,��ʱsend,post��ʧ�ܣ����Բ����е�make_shared<struct>������mSelfRefָ���Լ�������
 ����ͨHandler������ִ�����
- TimerManager�Ż�
 ��ס�ϴ�tick�Ͱ汾��
 ÿ��timer�б䶯ʱ�汾��+1
 ��TimerManager::ProcessTimer��tick��version�����ϴ���ͬʱ��˵�����ϴε�����ȫ��ͬ��ֱ�ӷ����ϴεĽ��
 ������ʡ�´������ظ����
 �汾��û�б仯ʱ����ֱ���û���tick��ȥ��ǰtick,����Ҫÿ�ζ�����ʱ����
- �Ľ�timer����
 ��windows������ʱ����ϸ΢Ư�ƣ�Linux�ºܾ�ȷ,ԭ��ûʱ���
- Handler������ʱpost��Ϣ��looper
 ����looper��quitʱ�Ͳ���Ҫsettimer����ʱ����Ƿ����quit looper��
 ֻ�е�Handler��parentΪlooperʱ����Ҫ������
 ���handler��parentҲ��handler,����Ҫpost��looper
- Ҫ���Ƕ������corelooper�Ŀ���ô����
 �ɲο�MFC ���dll�Ĵ���
 MFC��ÿ��extension dll����һ��app����Ȼ��.exe�Լ�Ҳ��һ��app,�����ڲ�����ô�����
- ����lambda��Э��
 ǰ����Ҫ���ּ����ã�����û�б�Ҫ
- �о�HandlerҪ����action chain�ĸ���
 ����������������һ��ҵ������̱Ƚ϶࣬��Ҫ�ֶ������
 �����������������Ҫ��ɢ�ڶ���ط�ת�Ӵ���,������Ϣ��תȥ,�������Ǻ�����
 action chain�ĺô��Ǽ�����һ���������ƶ����,ִ��ʱ�����Ͱ�
�ɲο��������Ŀ��
https://github.com/xhawk18/promise-cpp
https://github.com/Amanieu/asyncplusplus/wiki/Parallel-algorithms
- ���ڰ���app���Կ�looper����Handler��ȫ��
 ������һ��bit,��ͼ����ʱת������һ��Looper,���û�ܳɹ��������ٷ��ظ�ԭlooper
- ���󲿷�Handler���ܲ���Ҫname��shortcut,����tagHandlerInternalData���Ż�һ�£���һ��shared_ptr<>�ṹ��������Ҫʱ����
 ����ÿ��tagHandlerInternalData�ɽ�ʡ��ʮ���ֽ�

- �Ľ�DT����
һֱ�������DT.exe�����ڵ�ListCtrl��ΪRichEdit,��Android Studio�е�logcat����  
��Ҫ����֧�ֹ���  
- ��������
- ...
