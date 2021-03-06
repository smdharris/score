#pragma once
#include <score/selection/Selection.hpp>

#include <QPushButton>
#include <QString>
#include <QWidget>

class QPushButton;

namespace score
{
class SelectionDispatcher;
}
class SelectionButton final : public QPushButton
{
public:
  SelectionButton(
      const QString& text,
      Selection target,
      score::SelectionDispatcher& disp,
      QWidget* parent);

  template <typename Obj>
  static SelectionButton*
  make(Obj&& obj, score::SelectionDispatcher& disp, QWidget* parent)
  {
    return new SelectionButton{
        QString::number(*obj->id().val()), Selection{obj}, disp, parent};
  }

  template <typename Obj>
  static SelectionButton* make(
      const QString& text,
      Obj&& obj,
      score::SelectionDispatcher& disp,
      QWidget* parent)
  {
    auto but = new SelectionButton{text, Selection{obj}, disp, parent};

    but->setToolTip(QString::number(obj->id().val()));
    return but;
  }

private:
  score::SelectionDispatcher& m_dispatcher;
};
