namespace GoogleOAuthSample.Google
{
    public static class Google
    {
        public static GoogleUser Data { get; set; }
    }

    public class GoogleUser
    {
        public string ID { get; set; }

        public string AccessToken { get; set; }

        public string RefreshToken { get; set; }

        public string FirstName { get; set; }

        public string LastName { get; set; }

        public string Email { get; set; }

        public string ImageUrl { get; set; }
    }
}