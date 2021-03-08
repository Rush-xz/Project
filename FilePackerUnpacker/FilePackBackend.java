import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;/*nio subpackage native input output code become fast
                             nio package goes dirctly to the os       */
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;
import java.util.stream.Stream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.util.Arrays;

public class FilePackBackend 
{
    //stream is created for file handelling[object is not created yet]p
    FileOutputStream outstream = null;

    //file with following extensions are going to pack
    String ValidExt[] = { ".txt", ".c", ".java", ".cpp" };

    /* constructor
        src contains directory path
        dest contain name of file which we want to create
    */
    public FilePackBackend(String src, String Dest) throws Exception
    {
        String Magic = "Marvellous11";//its like primary header magic number

        byte arr[] = Magic.getBytes();

        //initialize File object and passing path as argument  
        File outfile = new File(Dest);
        File infile = null;
        //it's almost like a BufferWriter File created
        outstream = new FileOutputStream(Dest);
        //writing data in file [magic]
        outstream.write(arr, 0, arr.length);
        
        //opening src folder 
        File folder = new File(src);
        //setting current dirctory
        System.setProperty("user.dir", src);

        //list Files iterate
        listAllFiles(src);
    }

    public void listAllFiles(String path) 
    {   
        /*travelling through directory giving name of every file which we use for next activity
         Files is class contains walk method
        */
        try (Stream<Path> paths = Files.walk(Paths.get(path)))
        {
            //fetching path of every file
            paths.forEach(filePath -> 
            {
                //File should be regular .c,.txt not directory
                if (Files.isRegularFile(filePath)) //checking weather file is not dir.
                {//skiping non-regular file[not folder & shortcut]
                    try 
                    {
                        /*fetching only last name of file from path using getFileName()
                            path is like Demo\Hello\File.txt
                            it will give File.txt
                            converting using toString()
                        */
                        String name = filePath.getFileName().toString();

                        //file.txt it will give .txt(extensions)
                        String ext = name.substring(name.lastIndexOf("."));

                        //Converting array into list
                        List<String> list = Arrays.asList(ValidExt);

                        //checking received extension is present in ValidExt list
                        if (list.contains(ext)) 
                        {
                            File file = new File(filePath.getFileName().toString());

                            //getAbsolutePath gives path from starting full path
                            pack(file.getAbsolutePath());
                        }
                    } 
                    catch (Exception e) 
                    {
                        System.out.println(e);
                    }
                }
            });//end of foreach
        } 
        catch (Exception e) 
        {
            System.out.println(e);
        }
    }

    public void pack(String filePath) 
    {
        FileInputStream instream = null;
        try 
        {
            //byte array for buffer
            byte[] buffer = new byte[1024];
            int length;

            //Header
            byte[] temp = new byte[100];

            //trying to open the file
            File fobj = new File(filePath);

            //Filling header [use StringBuffer]
            String header = filePath + " " + fobj.length();

            //Filling remaining space in header with space
            for (int i = header.length(); i < 100; i++) 
            {
                header = header + " ";
            }
            temp = header.getBytes();//Fetching bytes from header

            //it is use to read the data from the file
            instream = new FileInputStream(filePath);

            //Writing header in file[outstream]
            outstream.write(temp, 0, temp.length);

            while((length = instream.read(buffer)) > 0) 
            {
                //writing data in new file[outstream]
                outstream.write(buffer, 0, length);
            }

            instream.close();

        } 
        catch (Exception e) 
        {

        }
    }
}