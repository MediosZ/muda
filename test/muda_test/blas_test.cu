#include <catch2/catch.hpp>
#include <muda/muda.h>
#include <muda/blas/blas.h>

using namespace muda;

TEST_CASE("scal_axpy", "[blas]")
{
    stream      s;
    blasContext ctx(s);

    host_vector<float> h_x_buffer(16);
    for(size_t i = 0; i < h_x_buffer.size(); i++)
        h_x_buffer[i] = float(i);

    device_buffer<float> x_buffer(s);
    x_buffer.copy_from(h_x_buffer);

    dense::vec<float> x(x_buffer.data(), x_buffer.size());

    device_buffer<float> y_buffer(s);
    y_buffer.copy_from(h_x_buffer);

    dense::vec<float> y(y_buffer.data(), y_buffer.size());

    on(ctx)
        .scal(3.0f, x)     // x = 3 * x
        .axpy(2.0f, x, y)  // y = 2 * x + y
        .wait();           //wait

    host_vector<float> hy;
    y_buffer.copy_to(hy);
    launch::wait_stream(s);

    host_vector<float> gt;
    gt = h_x_buffer;
    for(auto& v : gt)
        v *= 7;
    REQUIRE(hy == gt);
}

TEST_CASE("nrm2 copy")
{
    stream      s;
    blasContext ctx(s);

    host_vector<float> h_x_buffer(16);
    for(size_t i = 0; i < h_x_buffer.size(); i++)
        h_x_buffer[i] = float(i);

    device_buffer<float> x_buffer(s);
    x_buffer.copy_from(h_x_buffer);
    device_buffer<float> y_buffer(s, x_buffer.size());

    auto x = make_vec(x_buffer);
    auto y = make_vec(y_buffer);

    float nrm;
    on(ctx).copy(x, y).nrm2(y, nrm).wait();

    host_vector<float> gt     = h_x_buffer;
    float              gt_res = 0.0f;
    for(size_t i = 0; i < gt.size(); i++)
        gt[i] *= gt[i];
    for(size_t i = 0; i < gt.size(); i++)
        gt_res += gt[i];
    gt_res = std::sqrt(gt_res);
    REQUIRE(gt_res == nrm);
}
