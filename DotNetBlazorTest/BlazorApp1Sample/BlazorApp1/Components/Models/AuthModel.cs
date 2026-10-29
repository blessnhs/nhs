
using System.Security.Cryptography;
using System.Text;
using System.ComponentModel.DataAnnotations;

namespace BlazorApp1.Server.Models
    {
        public class User
        {
            public string Email { get; }
            public User(string email)
            {
                Email = email;
            }
        }
        public interface IUserDatabase
        {
            Task<User> AuthenticateUser(string email, string password);
            Task<User> AddUser(string email, string password);
        }
        public class UserDatabase : IUserDatabase
        {
            private readonly IWebHostEnvironment env;
            public UserDatabase(IWebHostEnvironment env) => this.env = env;
            private static string CreateHash(string password)
            {
                var salt = "997eff51db1544c7a3c2ddeb2053f052";
                var md5 = new HMACMD5(Encoding.UTF8.GetBytes(salt + password));
                byte[] data = md5.ComputeHash(Encoding.UTF8.GetBytes(password));
                return System.Convert.ToBase64String(data);
            }
            public async Task<User> AuthenticateUser(string email, string password)
            {
                if (string.IsNullOrEmpty(email) || string.IsNullOrEmpty(password))
                    return null;
                var path = System.IO.Path.Combine(env.ContentRootPath, "Users");
                if (!System.IO.Directory.Exists(path))
                    return null;
                path = System.IO.Path.Combine(path, email);
                if (!System.IO.File.Exists(path))
                    return null;
                if (await System.IO.File.ReadAllTextAsync(path) != CreateHash(password))
                    return null;
                return new User(email);
            }
            public async Task<User> AddUser(string email, string password)
            {
                try
                {
                    if (string.IsNullOrEmpty(email) || string.IsNullOrEmpty(password))
                        return null;
                    var path = System.IO.Path.Combine(env.ContentRootPath, "Users"); // NOTE: THIS WILL CREATE THE "USERS" FOLDER IN THE PROJECT'S FOLDER!!!
                    if (!System.IO.Directory.Exists(path))
                        System.IO.Directory.CreateDirectory(path); // NOTE: MAKE SURE THERE ARE CREATE/WRITE PERMISSIONS
                    path = System.IO.Path.Combine(path, email);
                    if (System.IO.File.Exists(path))
                        return null;
                    await System.IO.File.WriteAllTextAsync(path, CreateHash(password));
                    return new User(email);
                }
                catch
                {
                    return null;
                }
            }

        public class LoginResult
        {
            public string message { get; set; }
            public string email { get; set; }
            public string jwtBearer { get; set; }
            public bool success { get; set; }
        }
        public class LoginModel
        {
            [Required(ErrorMessage = "Email is required.")]
            [EmailAddress(ErrorMessage = "Email address is not valid.")]
            public string email { get; set; } // NOTE: email will be the username, too

            [Required(ErrorMessage = "Password is required.")]
            [DataType(DataType.Password)]
            public string password { get; set; }
        }
        public class RegModel : LoginModel
        {
            [Required(ErrorMessage = "Confirm password is required.")]
            [DataType(DataType.Password)]
            [Compare("password", ErrorMessage = "Password and confirm password do not match.")]
            public string confirmpwd { get; set; }
        }
    }
    }

