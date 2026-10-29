using BlazorApp1.Components;
using BlazorApp1.Server.Models;
using GoogleOAuthSample.Google;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Authentication.Cookies;

using Microsoft.AspNetCore.Authentication.JwtBearer; // NOTE: THIS LINE OF CODE IS NEWLY ADDED
using Microsoft.IdentityModel.Tokens; // NOTE: THIS LINE OF CODE IS NEWLY ADDED

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddRazorComponents()
    .AddInteractiveServerComponents();

// Google Login OAuth
builder.Services.AddAuthentication(CookieAuthenticationDefaults.AuthenticationScheme)
    .AddCookie(opt =>
    {
        opt.Cookie.Name = "GoogleOAuth";
    })
    .AddGoogle(opt =>
    {
        opt.ClientId = builder.Configuration["Authentication:Google:ClientId"];
        opt.ClientSecret = builder.Configuration["Authentication:Google:ClientSecret"];

        // OAuth 인증 시 어떤 권한을 포함할지를 정하는 Scope.
        // Scope URL은 Google에서 정의된 값을 넣어준다.
        opt.Scope.Add("openid");
        opt.Scope.Add("https://www.googleapis.com/auth/userinfo.email");
        opt.Scope.Add("https://www.googleapis.com/auth/userinfo.profile");

        // AccessType에 대한 정의는 찾아보면 자세히 나와있다.
        // default값은 online이며, RefreshToken을 얻기위해서는 offline으로 지정해줘야 함.
        opt.AccessType = "offline";

        // Code 발급 후 Token 취득까지 해당 Warpper가 처리해준다.
        // 이 이벤트는 Token까지 취득 후 발생한다.
        opt.Events.OnCreatingTicket = context =>
        {
             var user = new GoogleUser()
             {
                 ID = context.User.GetProperty("sub").ToString(),
                 AccessToken = context.AccessToken,
                 RefreshToken = context.RefreshToken,
                 FirstName = context.User.GetString("family_name"),
                 LastName = context.User.GetString("given_name"),
                 Email = context.User.GetString("email"),
                 ImageUrl = context.User.GetString("picture"),
             };

             Google.Data = user; // 인증을 통해 얻은 구글 유저 데이터를 저장하여 사용
       
            return Task.CompletedTask;
        };
    });

builder.Services.AddControllers();

var app = builder.Build();


// NOTE: end block

// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment())
{
    app.UseExceptionHandler("/Error", createScopeForErrors: true);
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}
app.UseStatusCodePagesWithReExecute("/not-found", createScopeForStatusCodePages: true);
app.UseHttpsRedirection();

app.UseAntiforgery();

app.UseAuthentication();
app.UseAuthorization();
app.MapControllers();

app.MapStaticAssets();
app.MapRazorComponents<App>()
    .AddInteractiveServerRenderMode();

Initializer.LoadResourceData();

app.Run();
