import SocketServer
from time import ctime


class TestRequestHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        print '...connected from:', self.client_address
        while True:
            data = self.request.recv(1024)
            if len(data) == 0:
                break
            log = '[%s:%s] %s' % (self.client_address, ctime(), data)
            with open('/mnt/shell/nblog.txt', 'a') as f:
                f.write(log+"\r\n")
            print log
        print '...disconnected from:', self.client_address


if __name__ == '__main__':
    testServer = SocketServer.ThreadingTCPServer(('172.18.250.47', 17799), TestRequestHandler)
    print 'waiting for connection...'
    testServer.serve_forever()
