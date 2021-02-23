import java.io.BufferedReader;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.util.Arrays;

/**
 * Inspired by https://docs.oracle.com/javase/tutorial/networking/datagrams/examples/QuoteServerThread.java
 */
public class Tjener
{
    private DatagramSocket _datagramSocket = null;
    private boolean _run = true;

    public Tjener() throws IOException
    {
        this._datagramSocket = new DatagramSocket(1234); //Velger bare en kortlevd port
    }

    public void start()
    {
        while (this._run) //Stopper å kjøre om det er en Exception
        {
            try
            {
                byte[] buffer = new byte[256];

                DatagramPacket datagramPacket = new DatagramPacket(buffer, buffer.length);
                this._datagramSocket.receive(datagramPacket); //Venter på en pakke

                String answer = calc(new String(buffer, 0, buffer.length)); //Lager svar på forespørselen fra klienten

                buffer = answer.getBytes(); //Gjør svar om til bytes
                this._datagramSocket.send(new DatagramPacket(buffer, buffer.length, datagramPacket.getAddress(), datagramPacket.getPort())); //Sender svar tilbake til klienten som sendte forespørselen
            }
            catch (IOException ioe)
            {
                ioe.printStackTrace();
                this._run = false;
            }
            catch (Exception e)
            {
                e.printStackTrace();
                this._run = false;
            }
        }
        this._datagramSocket.close();
    }

    /**
     * Tar inn en String. Deler den opp og sjekker om definert syntax er riktig, om ikke kastes IllegalArgumentException.
     * Om syntax er OK, regnes returverdien ut, og returneres, som en String.
     * Om Exceptions blir kastet og tatt imot, returneres feilmeldinger til bruker om hva som kan være feil.
     * @param message
     * @return
     */
    private String calc(String message)
    {
        String returnValue;
        try
        {
            String[] values = message.trim().split(" ");
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
            return "Noe uventet gikk galt, sjekk format";
        }
        return returnValue;
    }
}
