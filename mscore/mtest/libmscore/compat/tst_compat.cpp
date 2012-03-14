

#include <QtTest/QtTest>


class tst_Compat : public QObject
      {
      Q_OBJECT

   private slots:
      void compat();
      };


//---------------------------------------------------------
//   compat
//---------------------------------------------------------

void tst_Compat::compat()
      {
      int a = 5;
      QCOMPARE(a, 4);
      }

QTEST_MAIN(tst_Compat)
#include "tst_compat.moc"

