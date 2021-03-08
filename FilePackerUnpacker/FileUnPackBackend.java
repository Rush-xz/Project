import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.io.FileInputStream;
import java.io.FileOutputStream;

public class FileUnPackBackend 
{
    // FileOutputStream outstream = null;

    //src contain name of file to be create
    public FileUnPackBackend(String src) throws Exception 
    {
        unpck(src);
    }

    public void unpck(String filePath) throws Exception 
    {
        try 
        {
            FileInputStream instream = new FileInputStream(filePath);

            //for Fetching data 
            byte header[] = new byte[100];
            int length = 0;

            byte Magic[] = new byte[12];
            instream.read(Magic, 0, Magic.length);

            String Magicstr = new String(Magic);

            if (!Magicstr.equals("Marvellous11"))
            {
                throw new InvalidFileException("Invalid Packed File format");
            }

            while((length = instream.read(header, 0, 100)) > 0) 
            {
                String str = new String(header);//header comes 

                //break the header[gives Demo\a.txt 66 gives a.txt 66]
                String ext = str.substring(str.lastIndexOf("\\"));
                ext = ext.substring(1);

                /* \s space , \ = escape sequence = \\s*/
                String[] words = ext.split("\\s"); //splits
                //In java if space is not accepted then write \\s

                // words[0] = name of file[a.txt] , words[1] = size of file[66]

                String fileName = words[0];
                System.out.println(fileName);
                int size = Integer.parseInt(words[1]);//converted into int from string
                System.out.println(size);
                byte[] arr = new byte[size];//arr is buffer to copy data
                instream.read(arr, 0, size);//reading file
                //creating a new file
                FileOutputStream fout = new FileOutputStream((fileName));
                fout.write(arr, 0, size);//writing data from arr into file
            }
        } 
        catch (InvalidFileException obj) 
        {   
            //throws backward where it called[FileUnpackFront]
            throw new InvalidFileException("Invalid Packed File format");
        } 
        catch (Exception e) 
        {

        }
    }
}
