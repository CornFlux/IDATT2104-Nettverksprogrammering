import java.io.*;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.Charset;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Base64;
import java.util.IllegalFormatException;

public class ServerThread extends Thread
{
    private Socket socket;
    private ArrayList<ServerThread> serverThreads;
    private OutputStream outputStream;
    private final String rfc6455_DEFINED = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    private boolean running = true;

    public ServerThread(Socket socket, ArrayList<ServerThread> serverThreads) throws IOException
    {
        this.socket = socket;
        this.serverThreads = serverThreads;
        this.outputStream = this.socket.getOutputStream();
    }

    @Override
    public void run()
    {
        try
        {
            InputStream inputStream = this.socket.getInputStream();
            while (this.running)
            {
                byte[] fromClient = new byte[0];
                while (true)
                {
                    byte[] temp = new byte[0];
                    if (inputStream.available() != 0)
                    {
                        temp = this.getLineFromBuffer(inputStream);
                    }
                    if (temp.length == 0 && fromClient.length != 0)
                    {
                        break;
                    }
                    else if (temp.length != 0)
                    {
                        fromClient = this.addByteArrays(fromClient, temp);
                    }
                }
                System.out.println("Connected!");

                String stringFromClient = new String(fromClient, "UTF-8");
                System.out.println("From Client:\n" + stringFromClient);
                if (stringFromClient.contains("GET")) //Det er en GET-request
                {
                    String key = stringFromClient.substring(stringFromClient.indexOf("-Key: ") + 6, stringFromClient.indexOf("==") + 2); //Dette er ikke sikkert da det kan hende GET/--her-- inneholder samme

                    byte[] response = ("HTTP/1.1 101 Switching Protocols\r\n" + "Upgrade: websocket\r\n" + "Connection: Upgrade\r\n" + "Sec-WebSocket-Accept: " + this.genereateAcceptKey(key) + "\r\n\r\n").getBytes("UTF-8");

                    System.out.println("Respons:\n" + new String(response, "UTF-8"));
                    this.outputStream.write(response);
                }
                else //Ellers er det en message
                {
                    if (fromClient[0] != (byte)0b10000001) //Sjekker at klienten har sendt en textframe og at final-bit er satt, og at det ikke er noen extentions
                    {
                        throw new IllegalArgumentException("Første byte må være 0x81");
                    }
                    else if ((byte)(fromClient[1] & 0b10000000) != (byte)0b10000000) //Sjekker at meldingen er masker
                    {
                        throw new IllegalArgumentException("Mask-bit må være satt på meldinger til server");
                    }
                    else if ((byte)(fromClient[1] & 0b01111111) >= (byte)0b01111110) //Hvis payload er lik 126 eller 127 vil meldingen være lengere enn det vi skal håndtere
                    {
                        throw new IllegalArgumentException("Payload er for stor, for det vi skal håndtere");
                    }

                    byte payloadLength = (byte)(fromClient[1] & 0b01111111);
                    System.out.println("Payload: " + payloadLength);
                    byte[] mask = new byte[4];
                    mask[0] = fromClient[2];
                    mask[1] = fromClient[3];
                    mask[2] = fromClient[4];
                    mask[3] = fromClient[5];
                    int dataStart = 6; //1 byte til: FIN-bit og 3-bit reservert, 4-bit til opcode, 1 byte til mask-bit og Payload length og 4 byte til mask, derfor starter data (Payload) på 7. byte, så bytes[6]

                    for (int i = dataStart, maskCount = 0; i < dataStart + payloadLength; i++)
                    {
                        byte b = (byte)(fromClient[i] ^ mask[maskCount++ % 4]);
                        System.out.print((char)b);
                    }


                    byte[] answer = ("Kake er godt ja!").getBytes("UTF-8");

                    this.printToAllClients(answer);
                }
            }
        }
        catch (IOException ioe)
        {
            ioe.printStackTrace();
        }
        catch (IllegalArgumentException iae)
        {
            iae.printStackTrace();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            this.running = false;
            try
            {
                this.outputStream.close();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
            System.out.println(this.getName() + " stopped!");
        }

    }

    /**
     * Denne metoden er lik readLine() i en BufferedReader, bare at den er non-blocking.
     *
     * @param inputStream
     * @return
     * @throws IOException
     */
    private byte[] getLineFromBuffer(InputStream inputStream) throws IOException
    {
        byte[] bytesRecived = new byte[1500];
        byte b = -1;
        int count = 0;
        while (inputStream.available() != 0)
        {
            b = inputStream.readNBytes(1)[0]; //Leser en byte
            bytesRecived[count++] = b;
        }
        byte[] outputBytes = new byte[count];
        for (int i = 0; i < count; i++)
        {
            outputBytes[i] = bytesRecived[i];
        }
        return outputBytes;
    }

    private synchronized String genereateAcceptKey(String key) throws NoSuchAlgorithmException, UnsupportedEncodingException
    {
        MessageDigest sha1 = MessageDigest.getInstance("SHA-1");
        sha1.reset();
        sha1.update((key + this.rfc6455_DEFINED).getBytes("UTF-8"));
        byte[] base64 = Base64.getEncoder().encode(sha1.digest());

        return new String(base64, "UTF-8");
    }

    private synchronized void printToAllClients(byte[] message)
    {
        byte[] protocalBytes = {(byte)0b10000001, (byte)message.length};
        byte[] total = this.addByteArrays(protocalBytes, message);
        this.serverThreads.forEach(serverThread ->
        {
            try
            {
                if (this != serverThread)
                {
                    serverThread.outputStream.write(total, 0, total.length);
                }
            }
            catch (SocketException se)
            {
                //se.printStackTrace();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        });
    }


    private byte[] addByteArrays(byte[] array1, byte[] array2)
    {
        int len = (array1.length + array2.length); //Finne ut ny total lengde
        byte[] tempClient = array1.clone(); //Lager en kopi av fromClient
        array1 = new byte[len]; //Inisialiserer fromClient til ny lengde
        for (int i = 0; i < len; i++)
        {
            if (i < tempClient.length) //Om det er mer igjen i kopien av fromClient, legger det inn
            {
                array1[i] = tempClient[i];
            }
            else //Ellers hentes det nye fra temp
            {
                array1[i] = array2[i - tempClient.length];
            }
        }
        return array1;
    }
}