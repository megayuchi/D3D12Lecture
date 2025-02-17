#pragma once

void QCInit();
LARGE_INTEGER QCGetCounter();
float QCMeasureElapsedTick(LARGE_INTEGER CurCounter, LARGE_INTEGER PrvCounter);
LARGE_INTEGER QCCounterAddTick(LARGE_INTEGER Counter, float Tick);
LARGE_INTEGER QCCounterSubTick(LARGE_INTEGER Counter, float Tick);

