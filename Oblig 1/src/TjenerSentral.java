import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;

public class TjenerSentral
{
    private final int _ANT_TJENERE = 1;
    private Tjenere[] _tjenere = new Tjenere[this._ANT_TJENERE];
    private int _teller = 0;

    public TjenerSentral()
    {

    }

    public void start() throws IOException
    {
        final int PORT = 1250;

        try (ServerSocket serverSocket = new ServerSocket(PORT))
        {
            System.out.println("Sentralen er igang");
            while (true)
            {
                Socket connection = serverSocket.accept();
                if (connection != null)
                {
                    this._tjenere[this._teller % this._ANT_TJENERE] = new Tjenere(connection);
                    this._tjenere[this._teller % this._ANT_TJENERE].start();
                    this._teller++;
                }
            }
        }
    }
}
