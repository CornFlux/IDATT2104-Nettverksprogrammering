package nettverksprogrammering.oblig5.web;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;

import javax.swing.text.html.HTML;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

@org.springframework.stereotype.Controller
public class Controller
{

    //For å få opp svaret på nettsiden, om du har åpnet den som en local://--path--, må du skru av 'Local file restrictions' (i Safari Develop->Disable Local File Restrictions)

    @PostMapping(value = "/compile")
    public ResponseEntity<String> inputSourceCode(@RequestBody String input) throws IOException, InterruptedException
    {
        String decoded = URLDecoder.decode(input, StandardCharsets.UTF_8); //Gjør om input til String med format utf-8

        BufferedWriter bufferedWriter = new BufferedWriter(new FileWriter("docker/out.cpp")); //Velger hvilken fil å skrive til
        bufferedWriter.write(decoded); //Skriver kildekode til fil
        bufferedWriter.close(); //Lukker bufferedWriter

        Process process = Runtime.getRuntime().exec("docker build . -t temp", null, new File("docker/")); //Bygger docker image

        if (process.waitFor() == 0) //Sjekker om prosessen exit'a uten problemer, altså exitcode = 0
        {
            Process runDockerImage = Runtime.getRuntime().exec("docker run --rm temp"); //Kjører docker image'et
            if (runDockerImage.waitFor() != 0) //Om docker run ikke går feilfritt
            {
                return new ResponseEntity<>("Server error!", HttpStatus.INTERNAL_SERVER_ERROR); //Sender feilmeling tilbake
            }

            String outputFromDocker = new String(runDockerImage.getInputStream().readAllBytes()); //Leser av vanlig output fra docker
            return new ResponseEntity<>(outputFromDocker, HttpStatus.OK); //returner output koden
        }
        else //Om docker image ikke blir bygd
        {
            return new ResponseEntity<>("Compilation failed, check syntax!", HttpStatus.BAD_REQUEST); //Sender feilmeling tilbake
        }
    }
}
