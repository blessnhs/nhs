using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.AspNetCore.Authentication.Google;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Mvc;

namespace GoogleOAuthSample.Google
{
    [Route("/api/auth")]
    [ApiController]
    public class GoogleAuthController : ControllerBase
    {
        [HttpGet("google/login")]
        public ActionResult GoogleLogin()
        {
            var properties = new AuthenticationProperties()
            {
                RedirectUri = "/",

                // prompt=consent : 매번 권한 요청 및 승인 시 RefreshToken 획득
                // prompt=login : 최초 권한 승인만 RefreshToken 획득
                Parameters = { { "prompt", "consent" } },
            };
            return Challenge(properties, GoogleDefaults.AuthenticationScheme);
        }

        [HttpGet("google/logout")]
        public async Task<ActionResult> GoogleLogout()
        {
            await HttpContext.SignOutAsync(CookieAuthenticationDefaults.AuthenticationScheme);
            Google.Data = null;
            return LocalRedirect("/");
        }
    }
}