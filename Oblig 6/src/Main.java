import java.io.File;
import java.io.IOException;

public class Main
{
    public static void main(String[] args) throws IOException
    {
        Process process = Runtime.getRuntime().exec("node src/client.js");
        Server server = new Server(3001);
        server.start();

        process.destroy();
    }
}
