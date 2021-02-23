import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.Arrays;
import java.util.Scanner;

/**
 * Inspired by https://docs.oracle.com/javase/tutorial/networking/datagrams/examples/QuoteClient.java
 */
public class Klient
{
    public static void main(String[] args) throws IOException
    {
        Scanner scanner = new Scanner(System.in);
        System.out.println("Hva heter host'en?");
        String host = scanner.nextLine(); //Leser inn hostname
        DatagramSocket datagramSocket = new DatagramSocket();
        byte[] buffer;
        DatagramPacket datagramPacket;
        System.out.println("Skriv et regnestykke med +/- og jeg svarer. (tall +/- tall)");
        String input = scanner.nextLine(); //Leser inn regnestykket

        while (!input.equals(""))
        {
            buffer = input.getBytes(); //Gjør input fra bruker om til bytes
            datagramPacket = new DatagramPacket(buffer, buffer.length, InetAddress.getByName(host), 1234); //Legger input fra bruker inn i en packet (pakke) sammen med host addressen og en port
            datagramSocket.send(datagramPacket); //Sender en pakke
            buffer = new byte[256]; //Må endre størrelsen på buffer, da input.getBytes(), kan være for kort til å ha rom til svaret, kunne alternativt hardcode'et verdien der buffer.length er på neste linje
            datagramPacket = new DatagramPacket(buffer, buffer.length);
            datagramSocket.receive(datagramPacket); //Venter på en pakke, vil vente helt til den mottar en pakke

            String respons = new String(datagramPacket.getData(), 0, datagramPacket.getLength()); //Leser av innholdet i responsen (pakken vi fikk)
            System.out.println("Respons: " + respons);
            input = scanner.nextLine(); //Leser inn et nytt regnestykke
        }
        datagramSocket.close();
    }
}
