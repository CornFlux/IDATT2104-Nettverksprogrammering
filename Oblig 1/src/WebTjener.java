import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;

public class WebTjener
{
    public static void main(String[] args) throws IOException
    {
        final int PORT = 80;
        InputStreamReader inputStreamReader = null;
        BufferedReader bufferedReader = null;
        PrintWriter printWriter = null;
        ArrayList<String> messages = new ArrayList<>();


        try (ServerSocket serverSocket = new ServerSocket(PORT); Socket connection = serverSocket.accept())
        {
            System.out.println("Logg for tjenersiden. Nå venter vi...");

            inputStreamReader = new InputStreamReader(connection.getInputStream());
            bufferedReader = new BufferedReader(inputStreamReader);
            printWriter = new PrintWriter(connection.getOutputStream(), true);
            System.out.println("Connected to client");

            String message = bufferedReader.readLine();
            while (!message.equals(""))
            {
                messages.add(message);
                System.out.println("En klient skrev: " + message);
                message = bufferedReader.readLine();
            }

            printWriter.println(createHTML(messages));
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            printWriter.close();
            bufferedReader.close();
            inputStreamReader.close();
        }
    }

    private static String createHTML(ArrayList<String> messages)
    {
        String returString = "HTTP/1.0 200 OK\nContent-Type: text/html; charset=uft-8\n\n<html><body>\n<h1>En Hilsen</h1><ul>\n";

        for (String s : messages)
        {
               returString += "<li>" + s + "</li>\n";
        }

        return returString + "</ul></body></html>\n\n";
    }
}
