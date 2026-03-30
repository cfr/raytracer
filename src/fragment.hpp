
/*class alignas(std::hardware_destructive_interference_size) Fragment {
    Point from_;
    Point to_;
    Size size_;
    std::vector<Color> data_;
public:
    Fragment(Point from, Point to, Size size) : from_(from), to_(to), size_(size) {
        auto total = static_cast<size_t>(size.width * (to.y - from.y));
        data_.resize(total);
    }

    void set(Point pt, Color color) {
        data_[size_.width * (pt.y-from_.y) + pt.x] = color;
    }

    Color get(Point pt) {
        return data_[size_.width * (pt.y-from_.y) + pt.x];
    }

    Image::Iterator begin() {
        return {from_.x, from_.y, size_};
    }

    Image::Iterator end() {
        auto x = to_.x + 1;
        if (x >= size_.width) {
            return {0, to_.y+1, size_};
        }
        return {x, to_.y, size_};
    }
};

Fragment traceFragment(const Scene& scene, Point from, Point to) {
    Fragment frag(from, to, scene.image.size());
    auto caster = RayCaster{scene.camera, scene.image.size()};

    for(auto it = frag.begin(); it != frag.end(); ++it) {
        auto pix = *it;
        auto ray = caster.cast(pix);
        Color color = trace(ray, scene.camera.eye, scene, scene.depth);
        auto clamped = Color{glm::clamp(color, Color{0}, Color{1})};
        frag.set(pix, clamped);
    }
    return frag;
}


        auto nthreads = 16;
        if (argc > 2) nthreads = std::stoi(argv[2]);
        std::vector<std::thread> threads;
        std::vector<std::future<Fragment>> frags;
        auto size = scene.image.size();
        int pxPerTask = size.width*size.height/nthreads;
        std::println("size {}, {}; pxPerTask {}", size.width, size.height, pxPerTask);
        for(int i = 0; i < nthreads; i++) {
            Point from { i*pxPerTask % size.width, i*pxPerTask/size.width };
            Point to { size.width-1, (i+1)*pxPerTask/size.width-1 };
            std::println("from {}, {} ... to {}, {}", from.x, from.y, to.x, to.y);
            std::packaged_task<Fragment(Scene&, Point, Point)> task(traceFragment);
            std::future<Fragment> ff = task.get_future();
            frags.push_back(std::move(ff));
            threads.emplace_back(std::move(task), std::ref(scene), from, to);
            threads[i].detach();
        }
        for(auto& ff: frags) {
            auto frag = ff.get();
            for(auto it = frag.begin(); it != frag.end(); ++it) {
                auto pix = *it;
                image.set(pix, frag.get(pix));
            }
        }
*/

