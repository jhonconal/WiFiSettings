#include "PasswordDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

PasswordDialog::PasswordDialog(const QString &ssid, QWidget *parent)
    : QDialog(parent)
{
    setupUi(ssid);
}

QString PasswordDialog::password() const
{
    return m_passwordEdit->text();
}

void PasswordDialog::setupUi(const QString &ssid)
{
    setWindowTitle(tr("Enter Password"));
    setFixedSize(360, 220);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Style the dialog
    setStyleSheet(
        "QDialog {"
        "  background-color: #F2F2F7;"
        "  border-radius: 12px;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 20);
    mainLayout->setSpacing(16);

    // Title
    QLabel *titleLabel = new QLabel(tr("Enter the password for \"%1\"").arg(ssid), this);
    titleLabel->setStyleSheet(
        "font-size: 15px; font-weight: 600; color: #1c1c1e;"
    );
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Password field
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Password"));
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #d1d1d6;"
        "  border-radius: 10px;"
        "  padding: 10px 14px;"
        "  font-size: 14px;"
        "  background-color: white;"
        "  color: #1c1c1e;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #007AFF;"
        "}"
    );
    mainLayout->addWidget(m_passwordEdit);

    // Show password checkbox
    m_showPasswordCheck = new QCheckBox(tr("Show Password"), this);
    m_showPasswordCheck->setStyleSheet(
        "QCheckBox { font-size: 13px; color: #636366; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
    );
    mainLayout->addWidget(m_showPasswordCheck);

    connect(m_showPasswordCheck, &QCheckBox::toggled, this, [this](bool show) {
        m_passwordEdit->setEchoMode(show ? QLineEdit::Normal : QLineEdit::Password);
    });

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setStyleSheet(
        "QPushButton {"
        "  border: 1px solid #d1d1d6;"
        "  border-radius: 10px;"
        "  padding: 10px 24px;"
        "  font-size: 14px;"
        "  color: #007AFF;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E5E5EA;"
        "}"
    );

    m_connectButton = new QPushButton(tr("Connect"), this);
    m_connectButton->setDefault(true);
    m_connectButton->setEnabled(false);
    m_connectButton->setStyleSheet(
        "QPushButton {"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 10px 24px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  color: white;"
        "  background-color: #007AFF;"
        "}"
        "QPushButton:hover {"
        "  background-color: #0056CC;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #B0B0B5;"
        "}"
    );

    btnLayout->addWidget(m_cancelButton);
    btnLayout->addWidget(m_connectButton);
    mainLayout->addLayout(btnLayout);

    // Enable connect button only when password is entered
    connect(m_passwordEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_connectButton->setEnabled(text.length() >= 8); // WPA minimum 8 chars
    });

    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_connectButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, [this]() {
        if (m_connectButton->isEnabled()) {
            accept();
        }
    });

    m_passwordEdit->setFocus();
}
