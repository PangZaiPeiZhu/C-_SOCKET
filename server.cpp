#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include "iostream"
#include "stdio.h"
#include <vector>
using namespace std;
/*ʹ�ñ��ĵķ�ʽ���д���*/
//����ͷ
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;//���ݳ��� 
	short cmd;//����
};
//DataPackage
//����
struct Login : public DataHeader
{
	//DataHeader header;
	Login() 
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};
struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};
struct LoginOut : public DataHeader
{
	LoginOut()
	{
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}	
	char userName[32];
};
struct LoginOutResult : public DataHeader
{
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};
//�¿ͻ��˼���
struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
//����һ��ȫ�ֵ�����
vector<SOCKET> g_clients;

//����һ���������д���
int processor(SOCKET _cSocket)
{
	//ʹ�û���������������
	char szRecv[1024] = {};
	//5 ���տͻ�����������
	int nLen = recv(_cSocket, (char*)&szRecv, sizeof(DataHeader), 0);
	//��� �� �ְ�
	/*����ͷְ���������Ҫ�����ڷ���˽�������ʱһ�ν������ݹ��� �� ���̵����*/
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("�ͻ���<Socket = %d>�Ѿ��˳�, ���������\n", _cSocket);
		return -1;
	}
	//printf("�յ�����: %d ���ݳ��ȣ�%d\n", header.cmd, header.dataLength);
	/*�ж����յ�������*/ //��ͻ��˽����շ����ݵ������ʹ��
	//if (nLen > sizeof(DataHeader))
	//{
	//}
	switch (header->cmd)
	{
		case CMD_LOGIN:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login* login = (Login*)szRecv;
			printf("�յ��ͻ���<Socket = %d>����:CMD_LOGIN ���ݳ��ȣ�%d userName = %s passWord = %s\n",_cSocket, login->dataLength, login->userName, login->PassWord);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			//send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
			send(_cSocket, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginOut *loginout = (LoginOut*)szRecv;
			printf("�յ�����:CMD_LOGINOUT ���ݳ��ȣ�%d userName = %s\n", loginout->dataLength, loginout->userName);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginOutResult ret;
			//send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
			send(_cSocket, (char*)&ret, sizeof(LoginOutResult), 0);
		}
		break;
		default:
		{
			DataHeader header = { 0, CMD_ERROR };
			send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
	}
}

int main() 
{
	/*����Windows socket 2.x����*/
	//�汾��
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	//socket��������������
	WSAStartup(ver, &dat);
	//---------------------------
	//--��Socket API��������TCP�����
	//1������һ��socket  �׽��� ��windows�� linux��ָ����ָ��
	/*socket(
	_In_ int af,(��ʾʲô���͵��׽���)
	_In_ int type,(������)
	_In_ int protocol
	);*/
	//IPV4�������׽��� AF_INET
	//IPV6�������׽��� AF_INET6
	SOCKET _sock =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	char msgBuf[] = "Hello, I'm Server.\n";
	//2��bind �����ڽ��տͻ������ӵ�����˿�
	/*
	bind(
	_In_ SOCKET s,
	_In_reads_bytes_(namelen) const struct sockaddr FAR * name,
	_In_ int namelen
	);
	*/
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;//ipv4
	_sin.sin_port = htons(4567);//�˿ں� ����������������������Ͳ�ͬ �����Ҫ����ת�� ʹ�� host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");//��������ip��ַ INADDR_ANY�������е�Ip��ַ�����Է��� һ������
	//�п��ܰ�ʧ��
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) 
	{
		printf("���󣬰�����˿�ʧ��...\n");
	}
	else
	{
		printf("�󶨶˿ڳɹ�...\n");
	}

	//3��listen ��������˿�
	/*
	listen(
	_In_ SOCKET s,
	_In_ int backlog
	);*/
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("���󣬼�������˿�ʧ��...\n");
	}
	else 
	{
		printf("��������˿ڳɹ�...\n");
	}
	while (true)
	{
		// ������ BSD socket windows�ϵ�һ������������
		//linux ��ʾ��������1
		/*�����ͻ�������
		select(
		_In_ int nfds,
		_Inout_opt_ fd_set FAR * readfds,//�ɶ�
		_Inout_opt_ fd_set FAR * writefds,//��д
		_Inout_opt_ fd_set FAR * exceptfds,//�쳣
		_In_opt_ const struct timeval FAR * timeout//��ѯ�ӳ�
		);*/
		fd_set fdRead;//������(socket)����
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);//���fd_set�������͵����� ��ʵ���ǽ�fd_count ��Ϊ0
		FD_ZERO(&fdWrite);//������
		FD_ZERO(&fdExp);
		//typedef struct fd_set {
		//	u_int fd_count; ����              /* how many are SET? */
		//	SOCKET  fd_array[FD_SETSIZE(64)];   /* an array of SOCKETs */
		//} fd_set;

		FD_SET(_sock, &fdRead);//�����������뼯����
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		for (int n = (int)g_clients.size() - 1; n >= 0; n --)
		{
			FD_SET(g_clients[n], &fdRead);//����ɶ������в�ѯ �Ƿ��пɶ�����
		}
		//nfds��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ ���������� 
		//���������ļ����������ֵ+1 ��windows�������������д0
		//���һ������д��NULL��ʾһֱ�����ڴ˵ȴ�
		timeval t = {1, 0};//ʱ����� &t ���Ϊ1��
		//struct timeval {
		//long    tv_sec;         /* seconds */
		//long    tv_usec;        /* and microseconds */};
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		/*���Ϸ�ʽΪ������ʽ�����û�пͻ��˽��뽫�����ڴ˴�*/
		if (ret < 0)
		{
			printf("select���������\n");
			break;//��ʾ���� ����ѭ��
		}
		//������socket�ɶ��Ļ���ʾ �пͻ����Ѿ����ӽ�����
		if (FD_ISSET(_sock, &fdRead))//�ж��������Ƿ��ڼ�����
		{
			//����
			FD_CLR(_sock, &fdRead);
			//4��accept �ȴ����տͻ�������
			/*
			accept(
			_In_ SOCKET s,
			_Out_writes_bytes_opt_(*addrlen) struct sockaddr FAR * addr,
			_Inout_opt_ int FAR * addrlen
			);*/
			//accept �ȴ����ܿͻ�������
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSocket = INVALID_SOCKET;
			_cSocket = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSocket)
			{
				printf("���󣬽��յ���Ч�ͻ���SOCKET...\n");
			}
			else
			{
				//���¿ͻ��˼���Ⱥ���������û� ������������ ��������ɱ����
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					//���͸����е�ÿ���ͻ���
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_cSocket);
				printf("�¿ͻ��˼���: socket = %d IP = %s \n", _cSocket, inet_ntoa(clientAddr.sin_addr));//inet_ntoaת��Ϊ�ɶ���ַ
			}
			
		}
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (processor(fdRead.fd_array[n]) == -1)
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		//printf("����ʱ�䴦������ҵ��\n");
	}
	//�Է���һ�˳�����ʱ �������׽��ֽ�������
	for (size_t n = g_clients.size() - 1; n > 0; n --)
	{
		closesocket(g_clients[n]);
	}
	//5��send ��ͻ��˷���һ������
	/*send(
	_In_ SOCKET s,
	_In_reads_bytes_(len) const char FAR * buf,
	_In_ int len,
	_In_ int flags
	);*/
	//char msgBuf[] = "Hello, I'm Server.";
	//+1��ʾ����β��һ�����͹�ȥ
	//send(_cSocket, msgBuf, strlen(msgBuf) + 1, 0);

	//6���ر��׽���closesocket
	closesocket(_sock);

	//---------------------------
	WSACleanup();
	printf("���˳����������\n");
	getchar();
	return 0;
}