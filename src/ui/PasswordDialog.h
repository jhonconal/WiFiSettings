#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

/**
 * @brief Modal dialog for entering WiFi password.
 */
class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(const QString &ssid, QWidget *parent = nullptr);

    QString password() const;

private:
    QLineEdit *m_passwordEdit;
    QCheckBox *m_showPasswordCheck;
    QPushButton *m_connectButton;
    QPushButton *m_cancelButton;

    void setupUi(const QString &ssid);
};

#endif // PASSWORDDIALOG_H
