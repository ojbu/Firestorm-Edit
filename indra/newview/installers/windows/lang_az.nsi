; First is default
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

; Language selection dialog
LangString InstallerLanguageTitle  ${LANG_AZERBAIJANI} "Installer Language"
LangString SelectInstallerLanguage  ${LANG_AZERBAIJANI} "Please select the language of the installer"

; subtitle on license text caption
LangString LicenseSubTitleUpdate ${LANG_AZERBAIJANI} " Update"
LangString LicenseSubTitleSetup ${LANG_AZERBAIJANI} " Setup"

LangString MULTIUSER_TEXT_INSTALLMODE_TITLE ${LANG_AZERBAIJANI} "Installation Mode"
LangString MULTIUSER_TEXT_INSTALLMODE_SUBTITLE ${LANG_AZERBAIJANI} "Install for all users (requires Admin) or only for the current user?"
LangString MULTIUSER_INNERTEXT_INSTALLMODE_TOP ${LANG_AZERBAIJANI} "When you run this installer with Admin privileges, you can choose whether to install in (e.g.) c:\Program Files or the current user's AppData\Local folder."
LangString MULTIUSER_INNERTEXT_INSTALLMODE_ALLUSERS ${LANG_AZERBAIJANI} "Install for all users"
LangString MULTIUSER_INNERTEXT_INSTALLMODE_CURRENTUSER ${LANG_AZERBAIJANI} "Install for current user only"

; installation directory text
LangString DirectoryChooseTitle ${LANG_AZERBAIJANI} "Installation Directory" 
LangString DirectoryChooseUpdate ${LANG_AZERBAIJANI} "Select the Firestorm directory to update to version ${VERSION_LONG}.(XXX):"
LangString DirectoryChooseSetup ${LANG_AZERBAIJANI} "Select the directory to install Firestorm in:"

LangString MUI_TEXT_DIRECTORY_TITLE ${LANG_AZERBAIJANI} "Installation Directory"
LangString MUI_TEXT_DIRECTORY_SUBTITLE ${LANG_AZERBAIJANI} "Select the directory into which to install Firestorm:"

LangString MUI_TEXT_INSTALLING_TITLE ${LANG_AZERBAIJANI} "Installing Firestorm..."
LangString MUI_TEXT_INSTALLING_SUBTITLE ${LANG_AZERBAIJANI} "Installing the Firestorm viewer to $INSTDIR"

LangString MUI_TEXT_FINISH_TITLE ${LANG_AZERBAIJANI} "Installing Firestorm"
LangString MUI_TEXT_FINISH_SUBTITLE ${LANG_AZERBAIJANI} "Installed the Firestorm viewer to $INSTDIR."

LangString MUI_TEXT_ABORT_TITLE ${LANG_AZERBAIJANI} "Installation Aborted"
LangString MUI_TEXT_ABORT_SUBTITLE ${LANG_AZERBAIJANI} "Not installing the Firestorm viewer to $INSTDIR."

; CheckStartupParams message box
LangString CheckStartupParamsMB ${LANG_AZERBAIJANI} "Could not find the program '$INSTNAME'. Silent update failed."

; installation success dialog
LangString InstSuccesssQuestion ${LANG_AZERBAIJANI} "Start Firestorm now?"

; remove old NSIS version
LangString RemoveOldNSISVersion ${LANG_AZERBAIJANI} "Checking for old version..."

; check windows version
LangString CheckWindowsVersionDP ${LANG_AZERBAIJANI} "Checking Windows version..."
LangString CheckWindowsVersionMB ${LANG_AZERBAIJANI} "Firestorm only supports Windows Vista with Service Pack 2 and later.$\nInstallation on this Operating System is not supported. Quitting."
LangString CheckWindowsServPackMB ${LANG_AZERBAIJANI} "It is recomended to run Firestorm on the latest service pack for your operating system.$\nThis will help with performance and stability of the program."
LangString UseLatestServPackDP ${LANG_AZERBAIJANI} "Please use Windows Update to install the latest Service Pack."

; checkifadministrator function (install)
LangString CheckAdministratorInstDP ${LANG_AZERBAIJANI} "Checking for permission to install..."
LangString CheckAdministratorInstMB ${LANG_AZERBAIJANI} 'You appear to be using a "limited" account.$\nYou must be an "administrator" to install Firestorm.'

; checkifadministrator function (uninstall)
LangString CheckAdministratorUnInstDP ${LANG_AZERBAIJANI} "Checking for permission to uninstall..."
LangString CheckAdministratorUnInstMB ${LANG_AZERBAIJANI} 'You appear to be using a "limited" account.$\nYou must be an "administrator" to uninstall Firestorm.'

; checkcpuflags
LangString MissingSSE2 ${LANG_AZERBAIJANI} "This machine may not have a CPU with SSE2 support, which is required to run Firestorm ${VERSION_LONG}. Do you want to continue?"

; closesecondlife function (install)
LangString CloseSecondLifeInstDP ${LANG_AZERBAIJANI} "Waiting for Firestorm to shut down..."
LangString CloseSecondLifeInstMB ${LANG_AZERBAIJANI} "Firestorm can't be installed while it is already running.$\n$\nFinish what you're doing then select OK to close Firestorm and continue.$\nSelect CANCEL to cancel installation."
LangString CloseSecondLifeInstRM ${LANG_AZERBAIJANI} "Firestorm failed to remove some files from a previous install."

; closesecondlife function (uninstall)
LangString CloseSecondLifeUnInstDP ${LANG_AZERBAIJANI} "Waiting for Firestorm to shut down..."
LangString CloseSecondLifeUnInstMB ${LANG_AZERBAIJANI} "Firestorm can't be uninstalled while it is already running.$\n$\nFinish what you're doing then select OK to close Firestorm and continue.$\nSelect CANCEL to cancel."

; CheckNetworkConnection
LangString CheckNetworkConnectionDP ${LANG_AZERBAIJANI} "Checking network connection..."

; error during installation
LangString ErrorSecondLifeInstallRetry ${LANG_AZERBAIJANI} "Firestorm installer encountered issues during installation. Some files may not have been copied correctly."
LangString ErrorSecondLifeInstallSupport ${LANG_AZERBAIJANI} "Please reinstall viewer from https://www.firestormviewer.org/downloads/ and contact https://www.firestormviewer.org/support/ if issue persists after reinstall."

; ask to remove user's data files
LangString RemoveDataFilesMB ${LANG_AZERBAIJANI} "Delete settings and cache files in Documents and Settings folder?"

; delete program files
LangString DeleteProgramFilesMB ${LANG_AZERBAIJANI} "There are still files in your Firestorm program directory.$\n$\nThese are possibly files you created or moved to:$\n$INSTDIR$\n$\nDo you want to remove them?"

; uninstall text
LangString UninstallTextMsg ${LANG_AZERBAIJANI} "This will uninstall Firestorm ${VERSION_LONG} from your system."

; ask to remove protocol handler registry entries registry keys that still might be needed by other viewers that are installed
LangString DeleteRegistryKeysMB ${LANG_AZERBAIJANI} "Do you want to remove Firestorm as default handler for virtual world protocols?$\n$\nIt is recomended to keep registry keys if you have other versions of Firestorm installed."

; <FS:Ansariel> Ask to create protocol handler registry entries
LangString CreateUrlRegistryEntries ${LANG_AZERBAIJANI} "Do you want to register Firestorm as default handler for virtual world protocols?$\n$\nIf you have other versions of Firestorm installed, this will overwrite the existing registry keys."

; <FS:Ansariel> Optional start menu entry
LangString CreateStartMenuEntry ${LANG_AZERBAIJANI} "Create an entry in the start menu?"

; <FS:Ansariel> Application name suffix for OpenSim variant
LangString ForOpenSimSuffix ${LANG_AZERBAIJANI} "for OpenSimulator"

LangString DeleteDocumentAndSettingsDP ${LANG_AZERBAIJANI} "Deleting files in Documents and Settings folder."
LangString UnChatlogsNoticeMB ${LANG_AZERBAIJANI} "This uninstall will NOT delete your Firestorm chat logs and other private files. If you want to do that yourself, delete the Firestorm folder within your user Application data folder."
LangString UnRemovePasswordsDP ${LANG_AZERBAIJANI} "Removing Firestorm saved passwords."

LangString MUI_TEXT_LICENSE_TITLE ${LANG_AZERBAIJANI} "Vivox Voice System License Agreement"
LangString MUI_TEXT_LICENSE_SUBTITLE ${LANG_AZERBAIJANI} "Additional license agreement for the Vivox Voice System components."
LangString MUI_INNERTEXT_LICENSE_TOP ${LANG_AZERBAIJANI} "Please read the following license agreement carefully before proceeding with the installation:"
LangString MUI_INNERTEXT_LICENSE_BOTTOM ${LANG_AZERBAIJANI} "You have to agree to the license terms to continue with the installation."
