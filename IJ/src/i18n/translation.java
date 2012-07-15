package i18n;
import java.util.Locale; 
import java.util.ResourceBundle; 
import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.InputStream;
/** 
* 
* @author oeway 2009-7-29 21:17:42 
*/ 

public class translation { 
	static Locale locale = Locale.getDefault() ; //new Locale("zh","CN");// 指定中文环境//
	static ResourceBundle resb=  ResourceBundle.getBundle("lang",locale); 
	static boolean enable = false;
	static Properties props = new Properties();
	static OutputStream ops ;
	public static String tr(String input){
		if(isNumeric(input)|| isIncludeChinese(input))
			return input;
    	String text = input ;
    	try{
    		text= resb.getString(input);
    		
    	}catch(java.util.MissingResourceException e)
    	{
    		e.printStackTrace();
    		//appendProperties("c:/lang_zh_CN.properties",input,text);
    		//writeProperties("c:/2lang_zh_CN.properties",input,text);
    	}
    	//writeProperties("c:/lang_zh_CN.properties",input,text);
		return text;
    }
	public static boolean isNumeric(String str)
	{
		Pattern pattern = Pattern.compile("[0-9.]*");
		Matcher isNum = pattern.matcher(str);
		if( !isNum.matches() )
		{
			return false;
		}
		return true;
	}
    public static boolean isIncludeChinese(String str){
        if (str.isEmpty()) {
            return false;
        }
        Pattern pattern = Pattern
                .compile("^\\s*\\S*[\\u0391-\\uFFE5]+\\s*\\S*$");
        Matcher matcher = pattern.matcher(str);
        return matcher.find();
    } 
	public static void writeProperties(String filePath,String paraKey,String paraValue){
		Properties props = new Properties();
		try {
			InputStream fis = new FileInputStream(filePath); 
	        props.load(fis);
		} catch (Exception e) {
			e.printStackTrace();
			
		}finally{
			try{
		        props.setProperty(paraKey, paraValue);
		        OutputStream ops = new FileOutputStream(filePath);
		        props.store(ops, "Generated for translation");
			}catch(Exception e2){
				e2.printStackTrace();
			}
		}

	}
	public static void appendProperties(String filePath,String paraKey,String paraValue){
		Properties props = new Properties();
			try{
		        props.setProperty(paraKey, paraValue);
		        OutputStream ops = new FileOutputStream(filePath,true);
		        props.store(ops, "Generated for translation");
			}catch(Exception e2){
				e2.printStackTrace();
			}
	}	
	
}
