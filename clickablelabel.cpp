#include "clickablelabel.h"

ClickableLabel::ClickableLabel()
{

}

void ClickableLabel::mousePressEvent(QMouseEvent *event){
    emit clicked();
}
