#pragma once
#include "Document/Constraint/ViewModels/ConstraintHeader.hpp"

class TemporalConstraintHeader : public ConstraintHeader
{
    public:
        TemporalConstraintHeader():
            ConstraintHeader{}
        {
            this->setAcceptedMouseButtons(0);
        }

        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        int type() const override
        { return QGraphicsItem::UserType + 6; }
};
