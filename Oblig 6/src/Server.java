import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;


//https://gyawaliamit.medium.com/multi-client-chat-server-using-sockets-and-threads-in-java-2d0b64cad4a7
public class Server
{
    private final int PORT;
    private ArrayList<ServerThread> serverThreads;


    public Server(int port)
    {
        this.PORT = port;
        this.serverThreads = new ArrayList<>();
    }

    public void start()
    {
        try (ServerSocket serverSocket = new ServerSocket(this.PORT))
        {
            while (true)
            {
                Socket socket = serverSocket.accept();
                ServerThread serverThread = new ServerThread(socket, this.serverThreads);
                this.serverThreads.add(serverThread);
                serverThread.start();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {

        }
    }
}
