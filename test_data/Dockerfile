FROM mcr.microsoft.com/dotnet/aspnet:5.0

COPY bin/ App/
COPY example/books.db App/books.db
WORKDIR /App

# ENV DOTNET_EnableDiagnostics=0
ENTRYPOINT ["dotnet", "example.dll"]
