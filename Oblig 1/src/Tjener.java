import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class Tjener
{
    public static void main(String[] args) throws IOException
    {
        final int PORT = 1250;
        InputStreamReader inputStreamReader = null;
        BufferedReader bufferedReader = null;
        PrintWriter printWriter = null;


        try (ServerSocket serverSocket = new ServerSocket(PORT); Socket connection = serverSocket.accept())
        {
            System.out.println("Logg for tjenersiden. Nå venter vi...");

            inputStreamReader = new InputStreamReader(connection.getInputStream());
            bufferedReader = new BufferedReader(inputStreamReader);
            printWriter = new PrintWriter(connection.getOutputStream(), true);
            System.out.println("Connected to client");

            printWriter.println("Hei, du har kontakt med tjenersiden! Skriv et regne stykke med +/- og jeg gir deg svar. (tall +/- tall)");

            String message = bufferedReader.readLine();
            while (message != null)
            {
                System.out.println("En klient skrev: " + message);
                printWriter.println(calc(message));
                message = bufferedReader.readLine();
            }
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

    private static String calc(String message)
    {
        String returnValue;
        try
        {
            String[] values = message.split(" ");
            if (values.length > 3)
            {
                throw new IllegalArgumentException();
            }

            int tall1, tall2;
            tall1 = Integer.parseInt(values[0]);
            tall2 = Integer.parseInt(values[2]);

            if (values[1].equals("+"))
            {
                returnValue = String.valueOf(tall1 + tall2);
            }
            else if (values[1].equals("-"))
            {
                returnValue = String.valueOf(tall1 - tall2);
            }
            else
            {
                throw new IllegalArgumentException("Bruker har skrevet inn på feil format: '" + message + "'");
            }
        }
        catch (IllegalArgumentException iae)
        {
            return "Husk å skriv inn på riktig format: tall +/- tall";
        }
        catch (Exception e)
        {
            e.printStackTrace();
            return "Noe gikk galt, sjekk format";
        }
        return returnValue;
    }
}