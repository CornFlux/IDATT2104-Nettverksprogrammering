import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class Tjenere extends Thread
{
    private InputStreamReader _inputStreamReader = null;
    private BufferedReader _bufferedReader = null;
    private PrintWriter _printWriter = null;
    Socket _connection;


    public Tjenere(Socket connection)
    {
        this._connection = connection;
    }


    @Override
    public void run() throws IllegalArgumentException
    {
        try
        {
            System.out.println("Logg for tjenersiden. Nå venter vi...");

            this._inputStreamReader = new InputStreamReader(this._connection.getInputStream());
            this._bufferedReader = new BufferedReader(this._inputStreamReader);
            this._printWriter = new PrintWriter(this._connection.getOutputStream(), true);
            System.out.println("Connected to client");

            this._printWriter.println("Hei, du har kontakt med tjenersiden! Skriv et regne stykke med +/- og jeg gir deg svar. (tall +/- tall)");

            String message = this._bufferedReader.readLine();
            while (message != null)
            {
                System.out.println("En klient skrev: " + message);
                this._printWriter.println(calc(message));
                message = this._bufferedReader.readLine();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            assert this._printWriter != null;
            this._printWriter.close();
            try
            {
                this._bufferedReader.close();
                this._inputStreamReader.close();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    private static synchronized String calc(String message)
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
            System.out.println("Generell feil");
            e.printStackTrace();
            return "Noe gikk galt, sjekk format";
        }
        return returnValue;
    }
}
