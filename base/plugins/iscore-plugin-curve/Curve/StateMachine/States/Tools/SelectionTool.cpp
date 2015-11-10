#include "SelectionTool.hpp"
#include <iscore/statemachine/CommonSelectionState.hpp>
#include "Curve/StateMachine/CurveStateMachine.hpp"

#include "Curve/CurveModel.hpp"
#include "Curve/CurvePresenter.hpp"
#include "Curve/CurveView.hpp"

#include "Curve/Point/CurvePointModel.hpp"
#include "Curve/Point/CurvePointView.hpp"

#include "Curve/Segment/CurveSegmentModel.hpp"
#include "Curve/Segment/CurveSegmentView.hpp"

#include "Curve/StateMachine/OngoingState.hpp"
#include "Curve/StateMachine/CommandObjects/MovePointCommandObject.hpp"

#include <iscore/statemachine/StateMachineUtils.hpp>
#include <iscore/document/DocumentInterface.hpp>
#include <iscore/selection/SelectionStack.hpp>

#include <core/document/Document.hpp>

#include <QGraphicsScene>

namespace Curve
{
// TODO MOVEME
class SelectionState : public CommonSelectionState
{
    private:
        QPointF m_initialPoint;
        QPointF m_movePoint;

        const CurveStateMachine& m_parentSM;
        CurveView& m_view;

    public:
        SelectionState(
                iscore::SelectionStack& stack,
                const CurveStateMachine& parentSM,
                CurveView& view,
                QState* parent):
            CommonSelectionState{stack, &view, parent},
            m_parentSM{parentSM},
            m_view{view}
        {
        }

        const QPointF& initialPoint() const
        { return m_initialPoint; }
        const QPointF& movePoint() const
        { return m_movePoint; }

        void on_pressAreaSelection() override
        {
            m_initialPoint = m_parentSM.scenePoint;
        }

        void on_moveAreaSelection() override
        {
            m_movePoint = m_parentSM.scenePoint;
            auto rect = QRectF{m_view.mapFromScene(m_initialPoint),
                               m_view.mapFromScene(m_movePoint)}.normalized();

            m_view.setSelectionArea(rect);
            setSelectionArea(rect);
        }

        void on_releaseAreaSelection() override
        {
            m_view.setSelectionArea(QRectF{});
        }

        void on_deselect() override
        {
            dispatcher.setAndCommit(Selection{});
            m_view.setSelectionArea(QRectF{});
        }

        void on_delete() override
        {
            qDebug() << "1";
        }

        void on_deleteContent() override
        {
            qDebug() << "2";
        }

    private:
        void setSelectionArea(QRectF scene_area)
        {
            using namespace std;
            Selection sel;

            for(const auto& point : m_parentSM.presenter().points())
            {
                if(point.shape().translated(point.pos()).intersects(scene_area))
                {
                    sel.append(&point.model());
                }
            }

            for(const auto& segment : m_parentSM.presenter().segments())
            {
                if(segment.shape().translated(segment.pos()).intersects(scene_area))
                {
                    sel.append(&segment.model());
                }
            }

            dispatcher.setAndCommit(filterSelections(sel,
                                                     m_parentSM.model().selectedChildren(),
                                                     multiSelection()));
        }
};

SelectionAndMoveTool::SelectionAndMoveTool(CurveStateMachine& sm):
    CurveTool{sm, &sm}
{
    m_state = new Curve::SelectionState{
            iscore::IDocument::documentFromObject(m_parentSM.model())->selectionStack(),
            m_parentSM,
            m_parentSM.presenter().view(),
            &localSM()};

    localSM().setInitialState(m_state);

    {
        auto co = new MovePointCommandObject(&sm.presenter(), sm.commandStack());
        m_moveState = new Curve::OngoingState{*co, nullptr};

        m_moveState->setObjectName("MovePointState");

        make_transition<ClickOnPoint_Transition>(m_state,
                                                 m_moveState,
                                                 *m_moveState);

        m_moveState->addTransition(m_moveState,
                                  SIGNAL(finished()),
                                  m_state);

        localSM().addState(m_moveState);
    }

    localSM().start();
}

void SelectionAndMoveTool::on_pressed()
{
    m_prev = std::chrono::steady_clock::now();
    mapTopItem(itemUnderMouse(m_parentSM.scenePoint),
               [&] (const CurvePointView* point)
    {
        localSM().postEvent(new ClickOnPoint_Event(m_parentSM.curvePoint, point));
        m_nothingPressed = false;
    },
    [&] (const CurveSegmentView* segment)
    {
        localSM().postEvent(new ClickOnSegment_Event(m_parentSM.curvePoint, segment));
        m_nothingPressed = false;
    },
    [&] ()
    {
        localSM().postEvent(new Press_Event);
        m_nothingPressed = true;
    });
}

void SelectionAndMoveTool::on_moved()
{
    auto t = std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(t - m_prev).count() < 16)
    {
        return;
    }

    if (m_nothingPressed)
    {
        localSM().postEvent(new Move_Event);
    }
    else
    {
        mapTopItem(itemUnderMouse(m_parentSM.scenePoint),
                   [&] (const CurvePointView* point)
        {
            localSM().postEvent(new MoveOnPoint_Event(m_parentSM.curvePoint, point));
        },
        [&] (const CurveSegmentView* segment)
        {
            localSM().postEvent(new MoveOnSegment_Event(m_parentSM.curvePoint, segment));
        },
        [&] ()
        {
            localSM().postEvent(new MoveOnNothing_Event(m_parentSM.curvePoint, nullptr));
        });
    }
    m_prev = t;
}

void SelectionAndMoveTool::on_released()
{
    if(m_nothingPressed)
    {
        localSM().postEvent(new Release_Event); // select
        m_nothingPressed = false;

        return;
    }

    mapTopItem(itemUnderMouse(m_parentSM.scenePoint),
               [&] (const CurvePointView* point)
    {
        m_state->dispatcher.setAndCommit(
                    filterSelections(&point->model(),
                                     m_parentSM.model().selectedChildren(),
                                     m_state->multiSelection()));


        localSM().postEvent(new ReleaseOnPoint_Event(m_parentSM.curvePoint, point));
    },
    [&] (const CurveSegmentView* segment)
    {
        m_state->dispatcher.setAndCommit(
                    filterSelections(&segment->model(),
                                     m_parentSM.model().selectedChildren(),
                                     m_state->multiSelection()));

        localSM().postEvent(new ReleaseOnSegment_Event(m_parentSM.curvePoint, segment));
    },
    [&] ()
    {
        localSM().postEvent(new ReleaseOnNothing_Event(m_parentSM.curvePoint, nullptr));
    });
}


}
