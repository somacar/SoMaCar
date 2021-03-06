#include "ocr.hpp"

OCRTess::OCRTess() { }
OCRTess::OCRTess(bool downsize, int region, int group) {
    this->downsize = downsize;
    this->REGION = region;
    this->GROUP = group;

    for (int i = 0; i < 2; i++) {
        Ptr<ERFilter> er_filter1 = createERFilterNM1(loadClassifierNM1(DIR + TR_NM1), 8, 0.00015f, 0.13f, 0.2f, true, 0.1f);
        Ptr<ERFilter> er_filter2 = createERFilterNM2(loadClassifierNM2(DIR + TR_NM2), 0.5);
        this->erf1.push_back(er_filter1);
        this->erf2.push_back(er_filter2);
    }
}

void OCRTess::init(int num) {
    this->num = num;
    for (int o = 0; o < num; o++) {
        this->ocrs.push_back(OCRTesseract::create());
    }
}

void OCRTess::set(UMat u) {
    if (this->downsize && u.cols > MIN_OCR) resize(u, u, Size(MIN_OCR, MIN_OCR));
    bitwise_not(u, this->img); // 색 반전
}

bool OCRTess::loop() {
    bool f;
    int i = 0;
    do {
        f = detectAndRecog();
        if (f) break;
        this->img = this->img.t();
        flip(this->img, this->img, 1);
        i++;
    } while (i < 4);
    return f;
}

bool OCRTess::detectAndRecog() {
    UMat grey = UMat::zeros(this->img.rows + 2, this->img.cols + 2, CV_8UC1);
    cvtColor(this->img.clone(), grey, COLOR_RGB2GRAY);

    vector<UMat> channels;
    channels.clear();
    channels.push_back(grey);
    Mat m = 255 - grey.getMat(ACCESS_READ | ACCESS_WRITE);
    channels.push_back(m.getUMat(ACCESS_READ));

    vector<vector<ERStat>> regions(2);
    regions[0].clear();
    regions[1].clear();

    switch (this->REGION) {
        case REG_CSER: {
            parallel_for_(Range(0, (int) channels.size()), Parallel_extractCSER(channels, regions, this->erf1, this->erf2));
            break;
        }
        case REG_MSER: {
            vector<vector<Point> > contours;
            vector<Rect> bboxes;
            Ptr<MSER> mser = MSER::create(21, (int) (0.00002 * grey.cols * grey.rows), (int) (0.05 * grey.cols * grey.rows), 1, 0.7);
            mser->detectRegions(grey, contours, bboxes);
            if (contours.size() > 0)
                MSERsToERStats(grey, contours, regions);
            break;
        }
        default: {
            break;
        }
    }

    /*Text Recognition (OCR)*/
    vector<vector<Vec2i> > nm_region_groups;
    vector<Rect> nm_boxes;
    switch (this->GROUP) {
        case 0:
            erGrouping(this->img, channels, regions, nm_region_groups, nm_boxes, ERGROUPING_ORIENTATION_HORIZ);
            break;
        case 1:
        default:
            erGrouping(this->img, channels, regions, nm_region_groups, nm_boxes, ERGROUPING_ORIENTATION_ANY, DIR + TR_GRP, 0.5);
            break;
    }

    if (!nm_boxes.size() || nm_boxes.size() > 1) return false;

    vector<string> words_detection;
    float min_confidence1 = 51.f, min_confidence2 = 60.f;

    vector<UMat> detections;
    for (int i = 0; i < (int) nm_boxes.size(); i++) {
//        rectangle(this->out, nm_boxes[i].tl(), nm_boxes[i].br(), Scalar(255, 255, 0), 3);
        UMat group_img = UMat::zeros(this->img.rows + 2, this->img.cols + 2, CV_8UC1);
        er_draw(channels, regions, nm_region_groups[i], group_img);
        group_img = group_img(nm_boxes[i]);
        copyMakeBorder(group_img.clone(), group_img, 15, 15, 15, 15, BORDER_CONSTANT, Scalar(0));
        detections.push_back(group_img);
    }
    vector<string> outputs((int) detections.size());
    vector<vector<Rect> > boxes((int) detections.size());
    vector<vector<string> > words((int) detections.size());
    vector<vector<float> > confidences((int) detections.size());

    if (!detections.size() || detections.size() > 1) return false;

    for (int i = 0; i < (int) detections.size(); i = i + this->num) {
        Range r;
        if (i + this->num <= (int) detections.size()) r = Range(i, i + this->num);
        else r = Range(i, (int) detections.size());
        parallel_for_(r, Parallel_OCR<OCRTesseract>(detections, outputs, boxes, words, confidences, this->ocrs));
    }

    for (int i = 0; i < (int) detections.size(); i++) {
        outputs[i].erase(remove(outputs[i].begin(), outputs[i].end(), '\n'), outputs[i].end());
        if (outputs[i].size() < 3) {
            continue;
        }
        for (int j = 0; j < (int) boxes[i].size(); j++) {
            boxes[i][j].x += nm_boxes[i].x - 15;
            boxes[i][j].y += nm_boxes[i].y - 15;
            if ((words[i][j].size() < 2) || (confidences[i][j] < min_confidence1) ||
                ((words[i][j].size() == 2) && (words[i][j][0] == words[i][j][1])) ||
                ((words[i][j].size() < 4) && (confidences[i][j] < min_confidence2)) ||
                isRepetitive(words[i][j]))
                continue;
            words_detection.push_back(words[i][j]);
//            rectangle(this->out, boxes[i][j].tl(), boxes[i][j].br(), Scalar(255, 0, 255), 3);
//            Size word_size = getTextSize(words[i][j], FONT_HERSHEY_SIMPLEX, (double) scale_font, (int) (3 * scale_font), NULL);
//            rectangle(this->out, boxes[i][j].tl() - Point(3, word_size.height + 3), boxes[i][j].tl() + Point(word_size.width, 0), Scalar(255, 0, 255), -1);
//            putText(this->out, words[i][j], boxes[i][j].tl() - Point(1, 1), FONT_HERSHEY_SIMPLEX, scale_font, Scalar(255, 255, 255), (int) (3 * scale_font));
        }
    }

    if (!words_detection.size() || words_detection.size() > 1) return false;
    return (words_detection[0].compare(WORD) == 0);
}

void OCRTess::show(bool b) {
    if (b) imshow("ocr", this->img);
    else destroyWindow("ocr");
}

bool OCRTess::isRepetitive(const string &s) {
    int count = 0;
    int count2 = 0;
    int count3 = 0;
    int first = (int) s[0];
    int last = (int) s[(int) s.size() - 1];
    for (int i = 0; i < (int) s.size(); i++) {
        if ((s[i] == 'i') ||
            (s[i] == 'l') ||
            (s[i] == 'I'))
            count++;
        if ((int) s[i] == first)
            count2++;
        if ((int) s[i] == last)
            count3++;
    }
    return ((count > ((int) s.size() + 1) / 2) || (count2 == (int) s.size()) || (count3 > ((int) s.size() * 2) / 3));
}

void OCRTess::er_draw(vector<UMat> &channels, vector<vector<ERStat> > &regions, vector<Vec2i> group, UMat &segmentation) {
    for (int r = 0; r < (int) group.size(); r++) {
        ERStat er = regions[group[r][0]][group[r][1]];
        if (er.parent != NULL) { // deprecate the root region
            int newMaskVal = 255;
            int flags = 4 + (newMaskVal << 8) + FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
            floodFill(channels[group[r][0]], segmentation, Point(er.pixel % channels[group[r][0]].cols, er.pixel / channels[group[r][0]].cols),
                      Scalar(255), 0, Scalar(er.level), Scalar(0), flags);
        }
    }
}
