    vector<vector<int>> pathSum(TreeNode* root, int sum) {
        vector<vector<int>> r;

        if (!root) return r;

        queue <tuple< TreeNode*, vector<int>, int> > Q;
        Q.push({root, {}, sum});
        while (!Q.empty()) {
            auto p = Q.front();
            Q.pop();
            TreeNode* cur = std::get<0>(p);
            vector<int> path = std::get<1>(p);
            int curSum = std::get<2>(p);
            curSum -= cur->val;
            path.push_back(cur->val);
            if (curSum == 0) {
                if ((cur->left == nullptr) && (cur->right == nullptr)) {
                    r.push_back(path);
                }
            } 
            if (cur->left != nullptr) Q.push({cur->left, path, curSum});
            if (cur->right != nullptr) Q.push({cur->right, path, curSum});
        }
        return r;
    }