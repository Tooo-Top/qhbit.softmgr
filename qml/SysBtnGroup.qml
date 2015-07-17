import QtQuick 1.1

Row
{
    id: sysbtngroup
    spacing: 0

    signal skin
    signal feedback
    signal menu
    signal min
    signal close

    SysBtn
    {
        id:skin
        picHover: "qrc:/i/skin_hover.png"
        picNormal: "qrc:/i/skin_normal.png"
        picPressed: "qrc:/i/skin_pressed.png"
        onClicked:
        {
            console.log("skin btn clicked")
            sysbtngroup.skin()
        }
    }

    SysBtn
    {
        id:feedback
        picHover: "qrc:/i/feedback_hover.png"
        picNormal: "qrc:/i/feedback_normal.png"
        picPressed: "qrc:/i/feedback_pressed.png"
        onClicked:
        {
            console.log("feedback btn clicked")
            sysbtngroup.feedback()
        }
    }

    SysBtn
    {
        id:menu
        picHover: "qrc:/i/menu_hover.png"
        picNormal: "qrc:/i/menu_normal.png"
        picPressed: "qrc:/i/menu_pressed.png"
        onClicked:
        {
            console.log("menu btn clicked")
            sysbtngroup.menu()
        }
    }

    SysBtn
    {
        id:min
        picHover: "qrc:/i/min_hover.png"
        picNormal: "qrc:/i/min_normal.png"
        picPressed: "qrc:/i/min_pressed.png"
        onClicked:
        {
            console.log("min btn clicked")
            sysbtngroup.min()
        }
    }

    SysBtn
    {
        id:close
        picHover: "qrc:/i/close_hover.png"
        picNormal: "qrc:/i/close_normal.png"
        picPressed: "qrc:/i/close_pressed.png"
        onClicked:
        {
            console.log("close btn clicked")
            sysbtngroup.close()
        }
    }
}
