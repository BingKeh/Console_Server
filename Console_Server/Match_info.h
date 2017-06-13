#pragma once

#include "Client_info.h"

class Match_info
{
private:
	Client_info *client1;
	Client_info *client2;

	// ����״̬
	int Status;

	// �ִμ���
	int turn_count;

	// ����״̬
	int chess[10][10];

public:
	Match_info();
	~Match_info();
	void setP1(Client_info *c1);
	void setP2(Client_info *c2);
	Client_info* getP1();
	Client_info* getP2();
};

