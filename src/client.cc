#include <string.h>
#include <stdio.h>
#include <mutex>
#include <condition_variable>

#include "protocol/HttpMessage.h"
#include "protocol/HttpUtil.h"
#include "factory/WFTaskFactory.h"
#include "manager/WFFacilities.h"

#include "factory/Workflow.h"
#include "factory/WFTaskFactory.h"
#include "message.h"

#define REDIRECT_MAX    5
#define RETRY_MAX       2

using WFMyTcpTask = WFNetworkTask<MyRequest,
									 MyResponse>;
using tutorial_callback_t = std::function<void (WFMyTcpTask *)>;

using namespace protocol;

void httpCallback(WFHttpTask* task)
{
	HttpRequest* req = task->get_req();
	HttpResponse* resp = task->get_resp();
	int state = task->get_state();
	int error = task->get_error();

	switch (state)
	{
	case WFT_STATE_SYS_ERROR:
		fprintf(stderr, "system error: %s\n", strerror(error));
		break;
	case WFT_STATE_DNS_ERROR:
		fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
		break;
	case WFT_STATE_SSL_ERROR:
		fprintf(stderr, "SSL error: %d\n", error);
		break;
	case WFT_STATE_TASK_ERROR:
		fprintf(stderr, "Task error: %d\n", error);
		break;
	case WFT_STATE_SUCCESS:
		break;
	}

	if (state != WFT_STATE_SUCCESS)
	{
		fprintf(stderr, "Failed. Press Ctrl-C to exit.\n");
		return;
	}

	std::string name;
	std::string value;

	/* Print request. */
	fprintf(stderr, "%s %s %s\r\n", req->get_method(),
		req->get_http_version(),
		req->get_request_uri());

	HttpHeaderCursor req_cursor(req);

	while (req_cursor.next(name, value))
		fprintf(stderr, "%s: %s\r\n", name.c_str(), value.c_str());
	fprintf(stderr, "\r\n");

	/* Print response header. */
	fprintf(stderr, "%s %s %s\r\n", resp->get_http_version(),
		resp->get_status_code(),
		resp->get_reason_phrase());

	protocol::HttpHeaderCursor resp_cursor(resp);

	while (resp_cursor.next(name, value))
		fprintf(stderr, "%s: %s\r\n", name.c_str(), value.c_str());
	fprintf(stderr, "\r\n");

	/* Print response body. */
	const void* body;
	size_t body_len;

	resp->get_parsed_body(&body, &body_len);
	printf("rsp: %s\n", (const char*)body);
}


class MyFactory : public WFTaskFactory
{
public:
	static WFMyTcpTask *create_tcp_task(const std::string& host,
												unsigned short port,
												int retry_max,
												tutorial_callback_t callback)
	{
		using NTF = WFNetworkTaskFactory<MyRequest, MyResponse>;
		WFMyTcpTask *task = NTF::create_client_task(TT_TCP, host, port,
													   retry_max,
													   std::move(callback));
		task->set_keep_alive(30 * 1000);
		return task;
	}
};

static WFFacilities::WaitGroup wait_group(1);

int main(int argc, char *argv[])
{
	// --- http client
	printf("send http request\n");
	std::string url = "http://127.0.0.1:8098/hello";

	WFHttpTask* http_task = WFTaskFactory::create_http_task(url, REDIRECT_MAX, RETRY_MAX, httpCallback);
	protocol::HttpRequest* req = http_task->get_req();
	//req->add_header_pair("Accept", "*/*");
	//req->add_header_pair("User-Agent", "Wget/1.14 (linux-gnu)");
	//req->add_header_pair("Connection", "close");
	http_task->start();
	// TODO: wait group
	getchar();

	// --- tcp client
	printf("send tcp request\n");
	std::mutex mutex;
	std::condition_variable cond;
	bool finished = false;

	std::string tcp_host = "127.0.0.1";
	unsigned short tcp_port = 8099;

	std::function<void (WFMyTcpTask *task)> callback =
		[&tcp_host, tcp_port, &callback](WFMyTcpTask *task) {
		int state = task->get_state();
		int error = task->get_error();
		MyResponse *resp = task->get_resp();
		char buf[1024] = { 0 };
		void* body;
		size_t body_size;

		if (state != WFT_STATE_SUCCESS)
		{
			if (state == WFT_STATE_SYS_ERROR)
				fprintf(stderr, "SYS error: %s\n", strerror(error));
			else if (state == WFT_STATE_DNS_ERROR)
				fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
			else
				fprintf(stderr, "other error.\n");
			return;
		}

		resp->get_message_body_nocopy(&body, &body_size);
		std::string rsp((char*)body, body_size); // FIXME: have to copy the buffer to avoid tail unkown char
		if (body_size != 0)
			printf("Server Response size: %d, body: %s\n", (int)body_size, rsp.c_str());

		printf("Input next request string (Ctrl-D to exit): ");
		scanf("%[^\n]%*c", buf); // contain white space, read until \n
		body_size = strlen(buf);
		if (body_size > 0)
		{
			WFMyTcpTask *next;
			next = MyFactory::create_tcp_task(tcp_host, tcp_port, 0, callback);
			next->get_req()->set_message_body(buf, body_size);
			next->get_resp()->set_size_limit(4 * 1024);
			**task << next; /* equal to: series_of(task)->push_back(next) */
		}
		else
			printf("\n");
	};

	/* First request is connect task, is emtpy. We will ignore the server response. */
	WFMyTcpTask *tcp_task = MyFactory::create_tcp_task(tcp_host, tcp_port, 0, callback);
	tcp_task->get_resp()->set_size_limit(4 * 1024);
	Workflow::start_series_work(tcp_task, [&mutex, &cond, &finished](const SeriesWork *)
	{
		mutex.lock();
		finished = true;
		cond.notify_one();
		mutex.unlock();
	});

	std::unique_lock<std::mutex> lock(mutex);
	while (!finished)
		cond.wait(lock);

	lock.unlock();

	wait_group.wait();

	return 0;
}

