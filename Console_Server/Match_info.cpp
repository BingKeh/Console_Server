#include "Match_info.h"



Match_info::Match_info()
{
	this->Status = -1;
	this->turn_count = -1;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			this->chess[i][j] = -1;
		}
	}
}


Match_info::~Match_info()
{
}

void Match_info::setP1(Client_info * c1)
{
	this->client1 = c1;
}

void Match_info::setP2(Client_info * c2)
{
	this->client2 = c2;
}

Client_info * Match_info::getP1()
{
	return this->client1;
}

Client_info * Match_info::getP2()
{
	return this->client2;
}
