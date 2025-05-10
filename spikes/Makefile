

server:
	gcc -ggdb cjson/cJSON.c lsp_server.c -o lsp_server

client:
	gcc -ggdb cjson/cJSON.c lsp_client2.c -o lsp_client -lpthread

server_test:
	@printf 'Content-Length: 60\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}' \
		| ./lsp_server

