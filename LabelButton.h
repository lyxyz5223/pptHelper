#pragma once
#include <qlabel.h>
#include <qfile.h>
class LabelButton :
	public QLabel
{
	Q_OBJECT
public:
	~LabelButton() {}
	LabelButton(QWidget* parent = nullptr);
	void setFold(bool j) {
		if(j)
			setSvgFile((":/svg/svgs/arrow-ios-forward-outline.svg"));
		else
			setSvgFile((":/svg/svgs/arrow-ios-back-outline.svg"));
		folded = j;
	}
	bool isFolded() {
		return folded;
	}
	void setIconSize(QSize size) {
		iconSize = size;
	}
	void setIcon(QIcon icon) {
		this->icon = icon;
	}
	void setImage(QImage image) {
		this->image = image;
	}
	void setSvgFile(QString path) {
		svgFile.setFileName(path);
		if(!svgFile.isOpen())
			svgFile.open(QIODevice::ReadOnly | QIODevice::Text);
		svgStr = svgFile.readAll();
	}
protected:
	void mouseReleaseEvent(QMouseEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void paintEvent(QPaintEvent* e);
signals:
	void clicked();
private:
	QPoint pressPos = QPoint();
	bool folded = false;
	QImage image;
	QIcon icon;
	QSize iconSize;
	QFile svgFile;
	QString svgStr;
};

