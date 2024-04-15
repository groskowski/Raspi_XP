from http.server import HTTPServer, SimpleHTTPRequestHandler
import logging
import sys

class MyRequestHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        try:
            # Log the received request
            logging.info(f"Received GET request from {self.client_address}")
            logging.debug(f"Request headers: {self.headers}")

            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            self.wfile.flush()  # Flush the response headers

            # Define the HTML content
            html_content = """
            <html>
            <head>
                <title>Simple Web Server -----------</title>
            </head>
            <body>
                <h1>Welcome to the Simple Web Server!</h1>
                <p>This is a basic HTML page served by the Python web server.</p>
            </body>
            </html>
            """

            self.wfile.write(html_content.encode())

        except Exception as e:
            # Log any exceptions and send an error response
            logging.error(f"Error handling request: {str(e)}")
            self.send_error(500, f"Internal Server Error: {str(e)}")

    def handle_error(self, request, client_address):
        # Log any errors that occur during request handling
        logging.error(f"Error handling request from {client_address}: {request}")

def run_server(port=8000):
    # Configure logging
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

    server_address = ('', port)
    httpd = HTTPServer(server_address, MyRequestHandler)
    logging.info(f"Server running on port {port}")
    httpd.serve_forever()

if __name__ == '__main__':
    # Check if a port number is provided as a command-line argument
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    else:
        port = 8000

    run_server(port)