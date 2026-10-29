namespace BlazorApp1.Components
{
    static public class Initializer
    {
        static public void LoadResourceData()
        {
            try
            {
                BibleInfo.LoadKRV();
                BibleInfo.LoadKJV();
                Dic.LoadDic();
                //Hymn.LoadList();
                BibleInfo.LoadNIV();
                //BibleInfo.CheckValidate();
            }
            catch (Exception e)
            {
            }
        }
    }
}
