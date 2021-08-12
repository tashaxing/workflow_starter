#ifndef _TUTORIALMESSAGE_H_
#define _TUTORIALMESSAGE_H_

#include <stdlib.h>
#include "protocol/ProtocolMessage.h"

using namespace protocol;

class MyMessage : public ProtocolMessage
{
private:
	virtual int encode(struct iovec vectors[], int max);
	virtual int append(const void *buf, size_t size);

public:
	int set_message_body(const void *body, size_t size);

	void get_message_body_nocopy(void **body, size_t *size)
	{
		*body = this->body;
		*size = this->body_size;
	}

protected:
	char head[4];
	size_t head_received;
	char *body;
	size_t body_received;
	size_t body_size;

public:
	MyMessage()
	{
		this->head_received = 0;
		this->body = NULL;
		this->body_size = 0;
	}

	MyMessage(MyMessage&& msg);
	MyMessage& operator = (MyMessage&& msg);

	virtual ~MyMessage()
	{
		free(this->body);
	}
};

using MyRequest = MyMessage;
using MyResponse = MyMessage;

#endif

