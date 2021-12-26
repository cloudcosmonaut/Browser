#include "ipfs-daemon.h"

#include <glibmm/shell.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/main.h>
#include <iostream>
#include <unistd.h>

#ifdef LEGACY_CXX
#include <experimental/filesystem>
namespace n_fs = ::std::experimental::filesystem;
#else
#include <filesystem>
namespace n_fs = ::std::filesystem;
#endif
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * \brief Spawn the IPFS daemon in an async manner using Glib. If needed under Linux (under Windows, it tries to start IPFS anyway).
 * \return True if process is started, otherwise false.
 */
void IPFSDaemon::spawn()
{
    // Check for PID under UNIX
    int daemonPID = IPFSDaemon::getExistingPID();
    // Is IPFS Daemon already running?
    if (daemonPID > 0)
    {
        std::cout << "INFO: IPFS Daemon is already running. Do not start another IPFS process." << std::endl;
    }
    else
    {
        std::string command = IPFSDaemon::locateIPFSBinary();
        if (n_fs::exists(command))
        {
            std::cout << "INFO: Starting IPFS Daemon: " << command << "..." << std::endl;
            try
            {
                // IPFS command
                Glib::ArrayHandle<std::string> argv = Glib::shell_parse_argv(command + " daemon --init --migrate");

                // Spawn flags
                // Disable stdout/stderr to default terminal. Don't reaped the child automatically
                // We could add "Glib::SPAWN_SEARCH_PATH", if you want to search the binary in the PATH
                Glib::SpawnFlags flags = Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL | Glib::SPAWN_DO_NOT_REAP_CHILD;

                // Start IPFS, using spawn_async,
                // optionally we can use spawn_async_with_pipes(), to retrieve stdout & stderr to specified output buffers
                Glib::spawn_async(this->workingDir, argv, flags, Glib::SlotSpawnChildSetup(), &this->pid);
                this->isRunning = true;
                this->childWatchHandler = Glib::signal_child_watch().connect(sigc::mem_fun(*this, &IPFSDaemon::child_watch_exit), this->pid);
            }
            catch (Glib::SpawnError &error)
            {
                std::cerr << "ERROR: IPFS process could not be started. Reason: " << error.what() << std::endl;
            }
            catch (Glib::ShellError &error)
            {
                std::cerr << "ERROR: IPFS process could not be started. Reason: " << error.what() << std::endl;
            }
        }
        else
        {
            std::cerr << "ERROR: IPFS Daemon is not found. IPFS will not work!" << std::endl;
        }
    }
}

/**
 * \brief Stop process manually
 */
void IPFSDaemon::stop()
{
    if (!this->isRunning)
        return; // It's already stopped

    Glib::spawn_close_pid(pid);
    this->isRunning = false;
}

/**
 * \brief Exit signal handler for the process.
 * Emits the exited signal with the status code.
 */
void IPFSDaemon::child_watch_exit(Glib::Pid pid, int childStatus)
{
    std::cout << "WARN: IPFS Daemon exited, with status: " << childStatus << std::endl;
    Glib::spawn_close_pid(pid);

    // Disconnect from signal
    if (this->childWatchHandler.connected())
        this->childWatchHandler.disconnect();

    this->isRunning = false;

    // Emit exit signal with status code
    exited.emit(childStatus);
}

/**
 * \brief Get Process ID (PID)
 * \return PID
 */
int IPFSDaemon::getPID() const
{
    if (!this->isRunning)
        return 0;
#ifdef _WIN32
    return GetProcessId(pid);
#else
    return pid;
#endif
}

/**
 * \brief Try to locate the ipfs binary path (IPFS go server)
 * \return full path to the ipfs binary, empty string when not found
 */
std::string IPFSDaemon::locateIPFSBinary()
{
    std::string binaryName = "ipfs";
#ifdef _WIN32
    binaryName += ".exe";
#endif
#ifdef __linux__
    // Try data directory first, when Linux package is installed
    for (std::string data_dir : Glib::get_system_data_dirs())
    {
        std::vector<std::string> path_builder{data_dir, "libreweb-browser", "go-ipfs", binaryName};
        std::string ipfs_binary_path = Glib::build_path(G_DIR_SEPARATOR_S, path_builder);
        if (Glib::file_test(ipfs_binary_path, Glib::FileTest::FILE_TEST_IS_EXECUTABLE))
        {
            return ipfs_binary_path;
        }
    }
#endif
    std::string currentPath = n_fs::current_path().string();
    // When working directory is the current folder (for Windows)
    std::string ipfs_binary_path1 = Glib::build_filename(currentPath, binaryName);
    // When working directory is the build/bin folder (relative path), during the build (when package is not installed yet)
    std::string ipfs_binary_path2 = Glib::build_filename(currentPath, "../..", "go-ipfs", binaryName);
    if (Glib::file_test(ipfs_binary_path1, Glib::FileTest::FILE_TEST_IS_EXECUTABLE))
    {
        return ipfs_binary_path1;
    }
    else if (Glib::file_test(ipfs_binary_path2, Glib::FileTest::FILE_TEST_IS_EXECUTABLE))
    {
        return ipfs_binary_path2;
    }
    else
    {
        return "";
    }
}

/**
 * \brief Retrieve existing running IPFS PID for **UNIX only** (zero if non-existent)
 * \return Process ID (0 of non-existent)
 */
int IPFSDaemon::getExistingPID()
{
    int pid = 0;
#ifdef __linux__
    int exitCode = -3;
    std::string stdout;
    try
    {
        Glib::spawn_command_line_sync("pidof -s ipfs", &stdout, nullptr, &exitCode);
        // Process exists
        if (exitCode == 0)
        {
            pid = std::stoi(stdout);
        }
    }
    catch (Glib::SpawnError &error)
    {
        std::cerr << "ERROR: Could not check of running IPFS process. Reason: " << error.what() << std::endl;
    }
    catch (Glib::ShellError &error)
    {
        std::cerr << "ERROR: Could not check of running IPFS process. Reason: " << error.what() << std::endl;
    }
#endif
    return pid;
}

/**
 * \brief Determine if we need to kill any running IPFS process (UNIX only)
 * \return true if it needs to be terminated, otherwise false
 */
// Currently used:
/*
bool IPFSDaemon::shouldProcessTerminated()
{
#ifdef __linux__
    char pathbuf[1024];
    memset(pathbuf, 0, sizeof(pathbuf));
    std::string path = "/proc/" + std::to_string(this->pid) + "/exe";
    if (readlink(path.c_str(), pathbuf, sizeof(pathbuf) - 1) > 0)
    {
        char beginPath[] = "/usr/share/libreweb-browser";
        // If the begin path does not match (!= 0), return true,
        // meaning the process will be killed.
        bool shouldKill = (strncmp(pathbuf, beginPath, strlen(beginPath)) != 0);

        // Also check the IPFS version
        try {
            std::string expectedString = "version 0.11.0";
            std::string stdout;
            Glib::spawn_command_line_sync(path + " version", &stdout);
            // Current running IPFS version matches our IPFS version, keep process running afterall
            if (stdout.find(expectedString) != std::string::npos)
            {
                shouldKill = false;
            }
        }
        catch (Glib::SpawnError &error)
        {
            std::cerr << "ERROR: Could not check IPFS version. Reason: " << error.what() << std::endl;
        }
        catch (Glib::ShellError &error)
        {
            std::cerr << "ERROR: Could not check IPFS version. Reason: " << error.what() << std::endl;
        }
        return shouldKill;
    }
#endif
    return false; // fallback; do not kill
}*/