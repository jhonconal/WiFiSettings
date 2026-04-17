#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>

/**
 * @brief Custom iOS-style toggle switch widget.
 *
 * Renders a rounded toggle switch that animates between
 * on (green) and off (gray) states.
 */
class SwitchButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled)
    Q_PROPERTY(qreal position READ position WRITE setPosition)

public:
    explicit SwitchButton(QWidget *parent = nullptr);

    bool isChecked() const { return m_checked; }
    QSize sizeHint() const override;

public slots:
    void setChecked(bool checked);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_checked;
    qreal m_position; // 0.0 = off, 1.0 = on

    qreal position() const { return m_position; }
    void setPosition(qreal pos);
    void animateToggle();
};

#endif // SWITCHBUTTON_H
