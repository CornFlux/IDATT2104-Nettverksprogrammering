package nettverksprogrammering.oblig5;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.builder.SpringApplicationBuilder;
import org.springframework.boot.web.servlet.support.SpringBootServletInitializer;

@SpringBootApplication
public class Oblig5Application extends SpringBootServletInitializer
{
    @Override
    protected SpringApplicationBuilder configure(SpringApplicationBuilder application) {
        return application.sources(Oblig5Application.class);

    }

    public static void main(String[] args)
    {
        SpringApplication.run(Oblig5Application.class, args);
    }
}
