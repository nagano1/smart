; ModuleID = 'lib.cpp'
source_filename = "lib.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @_Z4funci(i32 %0) local_unnamed_addr #0 {
  %2 = mul i32 %0, 9
  %3 = add i32 %2, 871
  ret i32 %3
}

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @_Z3fibi(i32 %0) local_unnamed_addr #0 {
  %2 = icmp sgt i32 %0, 0
  br i1 %2, label %3, label %35

3:                                                ; preds = %1
  %4 = add i32 %0, -1
  %5 = and i32 %0, 7
  %6 = icmp ult i32 %4, 7
  br i1 %6, label %23, label %7

7:                                                ; preds = %3
  %8 = sub i32 %0, %5
  br label %9

9:                                                ; preds = %9, %7
  %10 = phi i32 [ 1, %7 ], [ %20, %9 ]
  %11 = phi i32 [ 0, %7 ], [ %19, %9 ]
  %12 = phi i32 [ %8, %7 ], [ %21, %9 ]
  %13 = add nsw i32 %10, %11
  %14 = add nsw i32 %13, %10
  %15 = add nsw i32 %14, %13
  %16 = add nsw i32 %15, %14
  %17 = add nsw i32 %16, %15
  %18 = add nsw i32 %17, %16
  %19 = add nsw i32 %18, %17
  %20 = add nsw i32 %19, %18
  %21 = add i32 %12, -8
  %22 = icmp eq i32 %21, 0
  br i1 %22, label %23, label %9

23:                                               ; preds = %9, %3
  %24 = phi i32 [ undef, %3 ], [ %20, %9 ]
  %25 = phi i32 [ 1, %3 ], [ %20, %9 ]
  %26 = phi i32 [ 0, %3 ], [ %19, %9 ]
  %27 = icmp eq i32 %5, 0
  br i1 %27, label %35, label %28

28:                                               ; preds = %23, %28
  %29 = phi i32 [ %32, %28 ], [ %25, %23 ]
  %30 = phi i32 [ %29, %28 ], [ %26, %23 ]
  %31 = phi i32 [ %33, %28 ], [ %5, %23 ]
  %32 = add nsw i32 %29, %30
  %33 = add i32 %31, -1
  %34 = icmp eq i32 %33, 0
  br i1 %34, label %35, label %28, !llvm.loop !2

35:                                               ; preds = %23, %28, %1
  %36 = phi i32 [ 1, %1 ], [ %24, %23 ], [ %32, %28 ]
  ret i32 %36
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.disable"}
