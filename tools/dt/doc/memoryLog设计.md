XiongWanPing 2020.02.10
# Memory Log�ŵ�
- �ɾ�������log��app������Ӱ��
- logʼ����ͬ����
- 
# Memory Logȱ��
- ռ���ڴ��һ��
- �������С������־��Ƶ������LogServer������ȡ��ʱ�����ܶ�ʧ�Ͼɵ���־
- ��Ҫ�����Ľ�����ȡ����־

���������ڴ��㹻���򲻻ᶪʧ��־

# Memory Log���
- header+TLV log items
- �����ڴ����ޣ�Ҫ֧�ּ��log��ʧ

# Memory Log����
## ���
- app,����Ҫ��ӡlog�ĳ���
- LogServer,����ɼ�app��log,�������ṩ��־����

## �������
app���������ڴ棬��ӡlog
LogServer��app������ͬһhost,��app�����Ĺ����ڴ�����ȡlog,�������ṩ��־����,����ͨ��websocket���������չʾ

## LogServer��μ��app��shared memory log
�����¼��ֿ��еķ�ʽ
- ���û��ֹ��Ǽ�app��Ϣ��LogServer,app����app.+pid����sm,LogServer��ʱö��process
- ��LogServer����shared memory,app����mutexͬ��ͳһ��ӡ����sm

# �ο�����
- g3log
- https://github.com/gabime/spdlog
- 