#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "factory/WFTaskFactory.h"

#include "protocol/HttpMessage.h"
#include "protocol/HttpUtil.h"
#include "server/WFServer.h"
#include "server/WFHttpServer.h"
#ifndef _WIN32
	#include <unistd.h>
#endif

#include "factory/Workflow.h"
#include "factory/WFTaskFactory.h"
//#include "server/WFServer.h"
#include "message.h"

// matrix related
typedef std::vector<std::vector<double>> Matrix;

struct MMInput
{
	Matrix a;
	Matrix b;
};

struct MMOutput
{
	int error;
	size_t m, n, k;
	Matrix c;
};

bool is_valid_matrix(const Matrix& matrix, size_t& m, size_t& n)
{
	m = n = 0;
	if (matrix.size() == 0)
		return true;

	m = matrix.size();
	n = matrix[0].size();
	if (n == 0)
		return false;

	for (const auto& row : matrix)
		if (row.size() != n)
			return false;

	return true;
}

void matrix_multiply(const MMInput* in, MMOutput* out)
{
	size_t m1, n1;
	size_t m2, n2;

	if (!is_valid_matrix(in->a, m1, n1) || !is_valid_matrix(in->b, m2, n2))
	{
		out->error = EINVAL;
		return;
	}

	if (n1 != m2)
	{
		out->error = EINVAL;
		return;
	}

	out->error = 0;
	out->m = m1;
	out->n = n2;
	out->k = n1;

	out->c.resize(m1);
	for (size_t i = 0; i < out->m; i++)
	{
		out->c[i].resize(n2);
		for (size_t j = 0; j < out->n; j++)
		{
			out->c[i][j] = 0;
			for (size_t k = 0; k < out->k; k++)
				out->c[i][j] += in->a[i][k] * in->b[k][j];
		}
	}
}

using MMTask = WFThreadTask<MMInput, MMOutput>;

using namespace algorithm;

void print_matrix(const Matrix& matrix, size_t m, size_t n)
{
	for (size_t i = 0; i < m; i++)
	{
		for (size_t j = 0; j < n; j++)
			printf("\t%8.2lf", matrix[i][j]);

		printf("\n");
	}
}

void callback(MMTask* task)
{
	auto* input = task->get_input();
	auto* output = task->get_output();

	assert(task->get_state() == WFT_STATE_SUCCESS);

	if (output->error)
		printf("Error: %d %s\n", output->error, strerror(output->error));
	else
	{
		printf("Matrix A\n");
		print_matrix(input->a, output->m, output->k);
		printf("Matrix B\n");
		print_matrix(input->b, output->k, output->n);
		printf("Matrix A * Matrix B =>\n");
		print_matrix(output->c, output->m, output->n);
	}
}

// http related
void processHttp(WFHttpTask* server_task)
{
	protocol::HttpRequest* req = server_task->get_req();

	const char* req_url = req->get_request_uri();
	const char* req_method = req->get_method();

	printf("http url: %s, method: %s\n", req_url, req_method);

	protocol::HttpResponse* resp = server_task->get_resp();
	long long seq = server_task->get_task_seq();
	protocol::HttpHeaderCursor cursor(req);
	std::string name;
	std::string value;
	char buf[8192];
	int len;

	/* Set response message body. */
	resp->append_output_body_nocopy("<html>", 6);
	len = snprintf(buf, 8192, "<p>%s %s %s</p>", req->get_method(),
		req->get_request_uri(), req->get_http_version());
	resp->append_output_body(buf, len);

	while (cursor.next(name, value))
	{
		len = snprintf(buf, 8192, "<p>%s: %s</p>", name.c_str(), value.c_str());
		resp->append_output_body(buf, len);
	}

	resp->append_output_body_nocopy("</html>", 7);

	/* Set status line if you like. */
	resp->set_http_version("HTTP/1.1");
	resp->set_status_code("200");
	resp->set_reason_phrase("OK");

	resp->add_header_pair("Content-Type", "text/html");
	resp->add_header_pair("Server", "Sogou WFHttpServer");
	if (seq == 9) /* no more than 10 requests on the same connection. */
		resp->add_header_pair("Connection", "close");

	/* print some log */
	char addrstr[128];
	struct sockaddr_storage addr;
	socklen_t l = sizeof addr;
	unsigned short port = 0;

	server_task->get_peer_addr((struct sockaddr*)&addr, &l);
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in* sin = (struct sockaddr_in*)&addr;
		inet_ntop(AF_INET, &sin->sin_addr, addrstr, 128);
		port = ntohs(sin->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, 128);
		port = ntohs(sin6->sin6_port);
	}
	else
		strcpy(addrstr, "Unknown");

	fprintf(stderr, "Peer address: %s:%d, seq: %lld.\n",
		addrstr, port, seq);
}

// tcp related
using WFMyTcpTask = WFNetworkTask<MyRequest, MyResponse>;
using WFMyTcpServer = WFServer<MyRequest, MyResponse>;

void processTcp(WFMyTcpTask* task)
{
	MyRequest* req = task->get_req();
	MyResponse* resp = task->get_resp();
	void* body;
	size_t size;
	size_t i;

	// FIXME: it seems the socket treat whitespace as frame seperator
	req->get_message_body_nocopy(&body, &size);
	std::string request((char*)body, size); // FIXME: have to copy the buffer to avoid tail unkown char
	printf("Request size: %d, body: %s\n", size, request.c_str()); 
	for (i = 0; i < size; i++)
		((char*)body)[i] = toupper(((char*)body)[i]);

	resp->set_message_body(body, size);
}

int main(int argc, char** argv)
{
	std::cout << "hello workflow cross platform" << std::endl;

	// --- matrix multiply
	using MMFactory = WFThreadTaskFactory<MMInput,
		MMOutput>;
	MMTask* task = MMFactory::create_thread_task("matrix_multiply_task",
		matrix_multiply,
		callback);
	auto* input = task->get_input();

	input->a = { {1, 2, 3}, {4, 5, 6} };
	input->b = { {7, 8}, {9, 10}, {11, 12} };

	std::mutex mutex;
	std::condition_variable cond;
	bool finished = false;

	Workflow::start_series_work(task,
		[&mutex, &cond, &finished](const SeriesWork*)
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

	// --- http echo server
	int http_port = 8098;
	WFHttpServer http_server(processHttp);
	if (http_server.start(http_port) == 0)
	{
		printf("http server listening at %d\n", http_port);
		getchar();
		http_server.stop();
	}
	else
	{
		perror("Cannot start http server");
		exit(1);
	}

	// --- tcp echo server
	int tcp_port = 8099;
	struct WFServerParams params = SERVER_PARAMS_DEFAULT;
	params.keep_alive_timeout = 0; // long connection
	params.request_size_limit = 4 * 1024;

	WFMyTcpServer server(&params, processTcp);
	if (/*server.start(AF_INET6, tcp_port) == 0 ||*/
		server.start(AF_INET, tcp_port) == 0) // FIXME: in windows maybe not support IPv6 default, just use IPv4
	{
		printf("tcp server listening at %d\n", tcp_port);
		getchar();
		server.stop();
	}
	else
	{
		perror("Cannot start tcp server");
		exit(1);
	}

	printf("final exit\n");

	return 0;
}