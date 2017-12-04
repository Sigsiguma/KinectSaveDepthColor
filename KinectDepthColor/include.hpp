#pragma once

#include <opencv2/opencv.hpp>

/*
--core--
画像・行列データ構造の提供
配列操作
XMLおよびYAML入出力
コマンドラインパーサー
ユーティリティ機能など

--highgui--
画像データのウィンドウ表示のためのモジュール
namedWindow(ウィンドウの生成)
imshow(ウィンドウに画像データの表示)
waitKey(キー入力の待機)
destroyAllWindows(ウィンドウの破棄)
などに必要

--imgcodecs--
静止画入出力を行うために用いる出力
imread(画像データをファイルから読み込む)
imwrite(画像データをファイルに書き出す)
などに必要

--imgproc--
基本的な画像処理のためのモジュール
ヒストグラム
画像フィルタリング
画像の幾何学変換
などに必要
*/

/*
--#pragma CV_LIBRARY(lib_name)--
ライブラリをリンクする処理をdefineしたもの
*/

#define CV_LIB_PREFIX comment(lib, "opencv_"

#ifdef _DEBUG
#define CV_LIB_SUFFIX CV_LIB_VERSION "d.lib")
#else
#define CV_LIB_SUFFIX CV_LIB_VERSION ".lib")
#endif

#define CV_LIB_VERSION CVAUX_STR(CV_MAJOR_VERSION)\
    CVAUX_STR(CV_MINOR_VERSION)\
    CVAUX_STR(CV_SUBMINOR_VERSION)

#define CV_LIBRARY(lib_name) CV_LIB_PREFIX CVAUX_STR(lib_name) CV_LIB_SUFFIX

#pragma CV_LIBRARY(core)
#pragma CV_LIBRARY(imgcodecs)
#pragma CV_LIBRARY(highgui)
#pragma CV_LIBRARY(imgproc)