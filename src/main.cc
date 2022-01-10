#include "ipfs-daemon.h"
#include "mainwindow.h"
#include "option-group.h"
#include "project_config.h"

#include <gtkmm/application.h>
#include <iomanip>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

/**
 * \brief Entry point of the app
 */
int main(int argc, char* argv[])
{
  // Set the command-line parameters option settings
  Glib::OptionContext context("LibreWeb Browser - Decentralized Web Browser");
  OptionGroup group;
  context.set_main_group(group);

  // Create the GTK application
  auto app = Gtk::Application::create();
  app->set_flags(Gio::ApplicationFlags::APPLICATION_NON_UNIQUE);

  // Parse the context
  try
  {
    context.parse(argc, argv);
    if (group.version)
    {
      std::cout << "LibreWeb Browser " << PROJECT_VER << std::endl;
      exit(EXIT_SUCCESS);
    }
  }
  catch (const Glib::Error& error)
  {
    std::cerr << "ERROR: Parse failure: " << error.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  // By default start the IPFS Daemon
  if (!group.disableIPFSDaemon)
  {
    IPFSDaemon ipfsDaemon;
    ipfsDaemon.spawn();
  }

  // Run the GTK window in the main thread
  MainWindow window(group.timeout);
  return app->run(window);
}

/*

TODO: Start IPFS process outside this application via installation?

Unix Fork code:

    pid_t child_pid = fork();
    if (child_pid == 0)
    {
        // Run by child process
        return IPFSProcess::startIPFSDaemon();
    }
    else if (child_pid > 0)
    {
        // Run the GTK window in the parent process (child_pid is the PID of child process)
        MainWindow window(group.m_timeout);
        int exitCode = app->run(window);

        // TODO: If we have multiple browsers running, maybe don't kill the IPFS daemon child process yet..?
        // For now, let's don't kill the IPFS process
        //kill(child_pid, SIGTERM);
        return exitCode;
    }
    else // PID < 0, error
    {
        printf("ERROR: fork failed.\n");
        return EXIT_FAILURE;
    }
*/
