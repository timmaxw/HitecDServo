# Installing and running DPC-11 software
The DPC-11 software may require some extra steps to get working. The following steps worked for me on Windows 10 Home (version 10.0.19042 build 19042).

1. In the Windows control panel, find "Turn Windows features on or off". Enable the feature called ".NET Framework 3.5 (includes .NET 2.0 and 3.0)".
2. Disable Windows driver signature verification. There are several ways of doing this. One way is as follows:
   * In the Start menu, click Power, then hold Shift while clicking the "Restart" button.
   * A screen will appear saying "Choose an option". Click on "Troubleshoot", then "Advanced options", then "Startup settings", then "Restart".
   * Your computer will restart. You'll see a screen saying "startup settings". Press 7 to select "Disable driver signature enforcement".
   * Windows will boot normally, but driver signature enforcement will be disabled until the next time you reboot. Note that you need to re-disable Windows driver signature verification every time you reboot your PC.
3. Follow the instructions that came with the DPC-11 to install the DPC-11 control software:
   * Download the DPC-11 installation file from https://hitecrcd.com/products/servos/programmers/dpc-11/product
   * Extract the .ZIP file into a directory.
   * Run the DPC-11_Setup installer.
   * The installer should work normally. If you get a message about needing "dotnet framework 2.0.50727", double-check that you ran Step 1 above.
4. Connect the DPC-11 dongle to your PC.
   * You will see a popup saying something like "CP2102 USB to UART Bridge Controller has been configured". However, the DPC-11 won't actually work yet. Proceed to the next step.
5. Open the Windows Device Manager and look for the dongle in the list.
   * It will appear under "Other devices" as "CP2102 USB to UART Bridge Controller".
   * Double-click on it to open the Properties window. The "Device Status" will say "The drivers for this device are not installed. (Code 28)"
   * Click "Update Driver".
   * "Browse my computer for drivers". Browse to the folder where you unpacked the DPC-11 software .ZIP file. Select the folder "HITECRCD_DPC-11 Driver Installer".
   * You'll get a Windows Security popup saying that the driver may be unsafe. Click "Install this driver anyway".
   * In the Windows Device Manager list, the DPC-11 should now appear under "Universal Serial Bus controllers" as "HITECRCD_DPC-11". If you double-click on it, the "Device Status" should say "This device is working properly."
6. Now, you can open the DPC-11 application and use it normally.
   * If the DPC-11 is connected, the app startup screen should show a green box saying "DPC-11 is connected".
   * If you instead see a red box saying "DPC-11 is not connected", double-check that you disabled Windows driver signature verification by following Step 2 above, and that you haven't rebooted your PC since the last time you disabled Windows driver signature verification. (And of course, double-check that the DPC-11 is actually connected!)
