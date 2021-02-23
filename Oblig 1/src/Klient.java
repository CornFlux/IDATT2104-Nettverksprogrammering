import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ConnectException;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Scanner;

public class Klient
{
    public static void main(String[] args) throws IOException
    {
        final int PORT = 1250;
        InputStreamReader inputStreamReader = null;
        BufferedReader bufferedReader = null;
        PrintWriter printWriter = null;

        Scanner scanner = new Scanner(System.in);
        try (Socket connection = new Socket("localhost", PORT)) //Kobler seg automatisk opp mot localhost
        {
            System.out.println("Connected to host");

            inputStreamReader = new InputStreamReader(connection.getInputStream());
            bufferedReader = new BufferedReader(inputStreamReader);
            printWriter = new PrintWriter(connection.getOutputStream(), true);

            String answer = bufferedReader.readLine();
            System.out.println("Tjener: " + answer);

            String message = scanner.nextLine();
            while (!message.equals(""))
            {
                printWriter.println(message);
                answer = bufferedReader.readLine();
                System.out.println("Tjener: " + answer);
                message = scanner.nextLine();
            }
        }
        catch (ConnectException ce)
        {
            System.out.println("Kunne ikke koble seg til tjener");
            ce.printStackTrace();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            //Kan få problmer med at .close() gir exception
            printWriter.close();
            bufferedReader.close();
            inputStreamReader.close();
        }
    }
}
