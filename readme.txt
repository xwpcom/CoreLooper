Ŀ¼˵��
demo		��ʾ����
depends		���������������
doc			�ĵ�
include		ͷ�ļ�
lib			���ɵ�lib��(Ŀǰû�õ�)
projects	vs,android stuido,xcode����
src			���Ĵ���

.��linux�±���
	git clone��c��build.shĿǰû���Դ�+xȨ�ޣ������о��Զ���Ȩ�޵İ취��Ŀǰ��Ҫ�ֹ�����
	chmod +x ./c
	chmod +x ./build.sh
	����./c
	��./build.sh����
	����������demo,����:
	cd corelooper/build/release/bin
	./timer
	
	��ʾ��������
	./build.sh VERBOSE=1
	
	����debug��
	BUILD_TYPE=Debug ./build.sh VERBOSE=1
	�ο� https://blog.csdn.net/liuweihui521/article/details/52556375
	��������:
	cd CoreLooper/build/Debug/bin
	gdb -tui timer
	b main��main�������öϵ�
	s�Ǹ��ٽ�����
	n��ִ����һ�����
	gdb����̫�鷳������vs����Щ

.��windows�±���
	��vs2017��projects\libcorelooper\corelooper.sln
	��vs2017��demo\CoreLooperDemo.sln
	����demo��ͨ��unit test������
	ֱ�ӱ���,֧��x86��x64

.��android studio�±���
	��android studio��projects����.as��β���ļ���ֱ�ӱ���
	����
	projects/libcorelooper.as
