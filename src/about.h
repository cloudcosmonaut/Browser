#ifndef ABOUT_H
#define ABOUT_H

#include <gtkmm/aboutdialog.h>
#include <gtkmm/image.h>

/**
 * \class About
 * \brief About dialog
 */
class About: public Gtk::AboutDialog
{
public:
    About();
    virtual ~About();

    void show_about();
    void hide_about(int response);
protected:
    Gtk::Image icon; /*!< The logo of the app */
};
#endif