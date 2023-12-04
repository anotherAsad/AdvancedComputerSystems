#include <vector>
#include <random>
#include <functional>

std::random_device rd;

struct tricoord {
	float x, y, z;
	
	tricoord(float x=0.0, float y=0.0, float z=0.0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

struct polygon {
	tricoord color;
	std::vector<tricoord> vertices;
};

polygon getRandomPolygon(tricoord base, int vertex_count) {
	polygon polygon_A;
	
	static std::default_random_engine generator;
	static std::normal_distribution<float> distribution(0.0,0.005);
	
	// get color
	polygon_A.color.x = distribution(generator) * 100;
	polygon_A.color.y = distribution(generator) * 100;
	polygon_A.color.z = distribution(generator) * 100;
	
	// get vertices
	for(int i=0; i<vertex_count; i++) {
		polygon_A.vertices.emplace_back(
			base.x + distribution(generator),		// x coord
			base.y + distribution(generator), 		// y coord
			base.z + distribution(generator)
		);
	}
	
	// return an entire polygon. Will be copied.
	return polygon_A;
}

polygon getRandomPolygon(tricoord base, int vertex_count, const std::function< float(float, float) > &eqn_lambda) {
	static std::default_random_engine generator;
	static std::normal_distribution<float> distribution(0.0,0.1);
	
	polygon polygon_A;
		
	// get color
	polygon_A.color.x = distribution(generator) * 10;
	polygon_A.color.y = distribution(generator) * 10;
	polygon_A.color.z = distribution(generator) * 10;
	
	// get vertices
	for(int i=0; i<vertex_count; i++) {
		float x = base.x + distribution(generator);
		float y = base.y + distribution(generator);
		float z = eqn_lambda(x, y);
		
		if(z == -1) {
			//i--;
			continue;
		} 
	
		polygon_A.vertices.emplace_back(x, y, z);
	}
	
	// return an entire polygon. Will be copied.
	return polygon_A;
}
