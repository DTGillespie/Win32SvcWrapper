### Summary
Win32SvcWrapper registers a specified binary executable as defined in the config.ini with the Windows Service Control Manager.

### Configuration Example
```ini
[Service]
Name="ExampleService"
Path="C:\Path\To\Service\Executable\ExampleService.exe"
```
Note: The config.ini must be located inside the working directory of the binary executable.
### Use
```cmd
sc create "ExampleService" binPath="C:\Path\To\Service\Wrapper\Win32SvcWrapper.exe"
```
