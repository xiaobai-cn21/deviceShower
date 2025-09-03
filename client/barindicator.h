#ifndef BARINDICATOR_H
#define BARINDICATOR_H

#include <QWidget>
#include <QColor>

class BarIndicator : public QWidget {
    Q_OBJECT
public:
    explicit BarIndicator(QWidget *parent = nullptr);
    void setRange(double mn, double mx);
    void setValue(double v);
    void setColor(const QColor &c);
    void setLabel(const QString &t);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double min, max, value;
    QColor color;
    QString label;
};

#endif // BARINDICATOR_H
