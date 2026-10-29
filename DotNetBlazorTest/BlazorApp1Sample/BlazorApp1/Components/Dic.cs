namespace BlazorApp1.Components
{
    static public class Dic
    {
        static public Dictionary<string, string> _dictionary = new Dictionary<string, string>();
        static public void LoadDic()
        {
            if (_dictionary.ContainsKey("A") == true)
                return;

            var system_path = System.IO.Directory.GetCurrentDirectory();

            string path = system_path + "/Components/Resource"; // 폴더 경로

            if (Directory.Exists(path))
            {
                // 모든 파일 경로 가져오기
                string[] files = Directory.GetFiles(path);

                // 특정 확장자만 가져오기 (예: .txt)
                // string[] files = Directory.GetFiles(path, "*.txt");

                foreach (var book in files)
                {
                    using (var reader = new System.IO.StreamReader(book))
                    {
                        try
                        {
                            if (reader == null)
                                continue;

                            string text;
                            while ((text = reader.ReadLine()) != null)
                            {
                                if (text == "" || text == null)
                                    continue;

                                string[] words = text.Split(':');
                                if (words == null || words.Length < 2)
                                {
                                    words = text.Split('[');
                                    if (words == null || words.Length < 2)
                                        continue;
                                }

                                string word = words[0].TrimEnd();

                                _dictionary[word] = words[1];
                            }
                        }
                        catch (Exception e)
                        {

                        }
                    }
                }
                foreach (string file in files)
                {

                }

            }
        }
          
    }
}
